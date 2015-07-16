//
//  NIMPRView+Events.m
//  NIMPR
//
//  Created by Alessandro Volz on 5/27/15.
//  Copyright (c) 2015 volz.io. All rights reserved.
//

#import "NIMPRView+Private.h"
#import "NIMPRView+Events.h"
#import "NIMPRMoveTool.h"
#import "NIMPRRotateTool.h"
#import "NIMPRZoomTool.h"
#import <NIBuildingBlocks/NIIntersection.h>
#import "NIMPRController.h"
#import "NIMPRAnnotationInteractionTool.h"
#import "NIImageAnnotation.h"
#import <objc/runtime.h>

@implementation NIMPRView (Events)

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)tool:(NIMPRTool*)tool sel:(SEL)sel event:(NSEvent*)event otherwise:(void(^)())block {
    NSString* ssel = NSStringFromSelector(sel);
    if ([ssel hasPrefix:@"rightMouse"] || [ssel hasPrefix:@"otherMouse"])
        ssel = [@"mouse" stringByAppendingString:[ssel substringFromIndex:NSMaxRange([ssel rangeOfString:@"Mouse"])]];
    
    if ([ssel isEqualToString:@"mouseDown:"]) {
        self.mouseDown = YES;
    } else if ([ssel isEqualToString:@"mouseUp:"])
        self.mouseDown = NO;

    SEL vsel = NSSelectorFromString([@"view:" stringByAppendingString:ssel]), orvsel = NSSelectorFromString([NSString stringWithFormat:@"view:%@otherwise:", ssel]);
    if ([tool respondsToSelector:orvsel]) {
        if ([[tool performSelector:orvsel withObjects:self:event:block] boolValue])
            return;
    } else if ([tool respondsToSelector:vsel])
        if ([[tool performSelector:vsel withObjects:self:event] boolValue])
            return;
    
    if (block)
        block();
    else if ([NIMPRView.superclass respondsToSelector:sel])
        [super performSelector:sel withObject:event];
}

- (void)mouseDown:(NSEvent*)event {
    [self.publicGlowingAnnotations removeAllObjects];
    [self tool:self.ltool sel:_cmd event:event otherwise:nil];
}

- (void)rightMouseDown:(NSEvent*)event {
    [self tool:self.rtool sel:_cmd event:event otherwise:^{
        [NSMenu popUpContextMenu:self.menu withEvent:event forView:self];
    }];
}

- (void)otherMouseDown:(NSEvent*)event {
    [self tool:self.ltool sel:_cmd event:event otherwise:nil];
}

- (void)mouseUp:(NSEvent*)event {
    [self tool:self.ltool sel:_cmd event:event otherwise:^{
        [self hover:event location:[self convertPoint:event.locationInWindow fromView:nil]];
        NIMPRView* view = [[self.window.contentView hitTest:event.locationInWindow] if:NIMPRView.class];
        if (view != self)
            [view hover:event location:[view convertPoint:event.locationInWindow fromView:nil]];
        else {
            if (event.clickCount == 2)
                self.ltcAtSecondClick = [self toolForLocation:[view convertPoint:event.locationInWindow fromView:nil] event:nil annotation:NULL];
            if (event.clickCount >= 2) {
                if (self.ltcAtSecondClick == NIMPRRotateAxisTool.class) {
                    if (event.clickCount == 2)
                        [self rotateToInitial];
                    else if (event.clickCount == 3)
                        [self.window.windowController rotateToInitial];
                } else if (self.ltcAtSecondClick == NIMPRMoveOthersTool.class) {
                    if (event.clickCount == 2)
                        [self.window.windowController moveToInitial];
                }
            }
        }
    }];
}

- (void)rightMouseUp:(NSEvent*)event {
    [self tool:self.rtool sel:_cmd event:event otherwise:nil];
}

- (void)otherMouseUp:(NSEvent*)event {
    [self tool:self.ltool sel:_cmd event:event otherwise:nil];
}

- (void)mouseMoved:(NSEvent*)event {
    [self tool:self.ltool sel:_cmd event:event otherwise:^{
        [self hover:event location:[self convertPoint:event.locationInWindow fromView:nil]];
    }];
}

- (void)mouseDragged:(NSEvent*)event {
    [self tool:self.ltool sel:_cmd event:event otherwise:nil];
    [super mouseDragged:event];
}

- (void)scrollWheel:(NSEvent*)event {
    [self tool:self.ltool sel:_cmd event:event otherwise:nil];
}

- (void)rightMouseDragged:(NSEvent*)event {
    [self tool:self.rtool sel:_cmd event:event otherwise:nil];
}

- (void)otherMouseDragged:(NSEvent*)event {
    [self tool:self.ltool sel:_cmd event:event otherwise:nil];
}

- (void)mouseEntered:(NSEvent*)event {
    [self.window makeFirstResponder:self];
    [self.window makeKeyAndOrderFront:self];
    [self tool:self.ltool sel:_cmd event:event otherwise:nil];
}

- (void)mouseExited:(NSEvent*)event {
    [self tool:self.ltool sel:_cmd event:event otherwise:^{
        [self hover:event location:[self convertPoint:event.locationInWindow fromView:nil]];
    }];
}

- (void)keyDown:(NSEvent*)event {
    [self tool:self.ltool sel:_cmd event:event otherwise:^{
        NIMPRToolTag tool = 0;
        
        if ((event.modifierFlags&NSDeviceIndependentModifierFlagsMask) == 0)
            switch ([event.characters.lowercaseString characterAtIndex:0]) {
                case 'w': {
                    tool = NIMPRToolWLWW;
                } break;
                case 'm': {
                    tool = NIMPRToolMove;
                } break;
                case 'z': {
                    tool = NIMPRToolZoom;
                } break;
                case 'r': {
                    tool = NIMPRToolRotate;
                } break;
            }
        else
        if ((event.modifierFlags&NSDeviceIndependentModifierFlagsMask) == NSCommandKeyMask)
            switch ([event.characters.lowercaseString characterAtIndex:0]) {
                case 'r': {
                    [self.window.windowController reset];
                } break;
            }
        
        if (tool)
            [self.window.windowController setLtoolTag:tool];
    }];
}

- (void)keyUp:(NSEvent*)event {
    [self tool:self.ltool sel:_cmd event:event otherwise:nil];
}

- (void)flagsChanged:(NSEvent*)event {
    [self tool:self.ltool sel:_cmd event:event otherwise:^{
        [self hover:event location:[self convertPoint:[self.window convertPointFromScreen:[NSEvent mouseLocation]] fromView:nil]];
    }];
}

- (void)hover:(NSEvent*)event location:(NSPoint)location {
    BOOL displayOverlays = ((event.modifierFlags&NSCommandKeyMask) == 0) || ((event.modifierFlags&NSShiftKeyMask) == NSShiftKeyMask);
    if ([self.window.windowController displayOverlays] != displayOverlays)
        [self.window.windowController setDisplayOverlays:displayOverlays];
    
    if (self.mouseDown)
        return;

    if (!event)
        event = [NSApp currentEvent];

    NIAnnotation* annotation = nil;
    Class ltc = [self toolForLocation:location event:event annotation:&annotation];

    if (annotation && [self.publicGlowingAnnotations containsObject:annotation])
        [self.publicGlowingAnnotations intersectSet:[NSSet setWithObject:annotation]];
    else {
        [self.publicGlowingAnnotations removeAllObjects];
        if (annotation)
            [self.publicGlowingAnnotations addObject:annotation];
    }
    
    if (self.ltool.class != ltc)
        self.ltool = [[[ltc alloc] init] autorelease];
    
    [self enumerateIntersectionsWithBlock:^(NSString* key, NIIntersection* intersection, BOOL* stop) {
        intersection.maskAroundMouse = !ltc;
    }];
    
    [NIMPRTool setCursor:(NSPointInRect(location, self.bounds)? [self.ltool cursors][0] : nil)];
    
    Class rtc = nil;
    
    if (ltc == NIMPRMoveOthersTool.class)
        rtc = NIMPRCenterZoomTool.class;
    
    if (self.rtool.class != rtc)
        self.rtool = [[[rtc alloc] init] autorelease];
}

- (Class)toolForLocation:(NSPoint)location event:(NSEvent*)event annotation:(NIAnnotation**)rannotation {
    if (event.type == NSMouseExited)
        return nil;
    if ((event.modifierFlags&NSDeviceIndependentModifierFlagsMask) == NSCommandKeyMask)
        return nil;
    
    CGFloat distance;
    NSString* ikey = [self intersectionClosestToPoint:location closestPoint:NULL distance:&distance];
    
    BOOL rotate = (ikey && distance <= 4);
    
    __block BOOL move, cmove = move = rotate;
    if ([self.window.windowController spacebarIsDown])
        move = YES;
    else if (cmove)
        [self enumerateIntersectionsWithBlock:^(NSString* key, NIIntersection* intersection, BOOL* stop) {
            if ([key isEqualToString:ikey])
                return;
            if ([intersection distanceToPoint:location closestPoint:NULL] > 6) {
                cmove = move = NO;
                *stop = YES;
            }
        }];
    else if ((event.modifierFlags&NSDeviceIndependentModifierFlagsMask) == NSAlternateKeyMask)
        rotate = YES;
    
    Class ltc = nil;
    
    if (cmove)
        ltc = NIMPRMoveOthersTool.class;
    else if (move)
        ltc = NIMPRMoveTool.class;
    else if (rotate) {
        ltc = NIMPRRotateAxisTool.class;
        if ((event.modifierFlags&NSDeviceIndependentModifierFlagsMask) == NSAlternateKeyMask)
            ltc = NIMPRRotateTool.class;
    }
    
    if (!ltc) {
        NIAnnotation* annotation = [self annotationForLocation:location];
        if (annotation)
            ltc = NIMPRAnnotationInteractionTool.class;
        if (rannotation)
            *rannotation = annotation;
    }
    
    return ltc;
}

- (NIAnnotation*)annotationForLocation:(NSPoint)location {
    NIAnnotation* annotation = nil;
    
    if (NSPointInRect(location, self.bounds))
        for (size_t i = 0; i < 2; ++i) { // first try by filtering out image annotations, then with them
            CGFloat distance;
            
            if (!i)
                annotation = [self annotationClosestToPoint:location closestPoint:NULL distance:&distance filter:^BOOL(NIAnnotation* annotation) {
                    return ![annotation isKindOfClass:NIImageAnnotation.class];
                }];
            else
                annotation = [self annotationClosestToPoint:location closestPoint:NULL distance:&distance];
            
            if (annotation && distance > NIAnnotationDistant)
                annotation = nil;
            
            if (annotation)
                break;
        }
    
    return annotation;
}

@end
