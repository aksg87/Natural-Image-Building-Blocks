//
//  NIBezierPathAnnotation.m
//  NIMPR
//
//  Created by Alessandro Volz on 7/9/15.
//  Copyright (c) 2015 volz.io. All rights reserved.
//

#import "NIBezierPathAnnotation.h"
#import "NSBezierPath+NIMPR.h"

@implementation NIBezierPathAnnotation

+ (NSSet*)keyPathsForValuesAffectingAnnotation {
    return [super.keyPathsForValuesAffectingAnnotation setByAddingObject:@"NIBezierPath"];
}

- (NIBezierPath*)NIBezierPath {
    [NSException raise:NSInvalidArgumentException format:@"Method -[%@ NIBezierPath] must be implemented for all NIBezierPathAnnotation subclasses", self.className];
    return nil;
}

+ (NIBezierPath*)bezierPath:(NIBezierPath*)path minmax:(CGFloat)mm complete:(BOOL)complete {
    NIMutableBezierPath* mpath = [[path mutableCopy] autorelease];
    [mpath addEndpointsAtIntersectionsWithPlane:NIPlaneMake(NIVectorMake(0, 0, mm), NIVectorZBasis)];
    [mpath addEndpointsAtIntersectionsWithPlane:NIPlaneMake(NIVectorMake(0, 0, -mm), NIVectorZBasis)];

    NIMutableBezierPath* rpath = [NIMutableBezierPath bezierPath];
    
    NIVector ip, bp; BOOL ipset = NO, bpin = NO;
    NIVector c1, c2, ep;
    NSInteger elementCount = mpath.elementCount;
    NIBezierPathElement e;
    for (NSInteger i = 0; i < elementCount; ++i)
        switch (e = [mpath elementAtIndex:i control1:&c1 control2:&c2 endpoint:&ep]) {
            case NIMoveToBezierPathElement: {
                bp = ep; bpin = NO;
            } break;
            case NILineToBezierPathElement:
            case NICurveToBezierPathElement: {
                CGFloat mpz = (bp.z+ep.z)/2;
                if (mpz <= mm && mpz >= -mm) {
                    if (!bpin) {
                        if (!ipset) {
                            ip = bp; ipset = YES; }
                        if (!complete || !rpath.elementCount)
                            [rpath moveToVector:bp];
                        else [rpath lineToVector:bp]; }
                    if (e == NILineToBezierPathElement)
                        [rpath lineToVector:ep];
                    else [rpath curveToVector:ep controlVector1:c1 controlVector2:c2];
                    bpin = YES;
                } else
                    bpin = NO;
                bp = ep;
            } break;
            case NICloseBezierPathElement: {
                if (ipset) {
                    CGFloat mpz = (bp.z+ip.z)/2;
                    if (mpz <= mm && mpz >= -mm) {
                        if (!bpin) {
                            if (!complete || !rpath.elementCount)
                                [rpath moveToVector:bp];
                            else [rpath lineToVector:bp]; }
                        if (complete) {
                            [rpath lineToVector:ip];
                            bp = ip;
                        } else
                            [rpath close];
                        bpin = YES;
                    } else
                        bpin = NO;
                    bp = ip;
                }
            } break;
        }
    
    return rpath;
}

- (NIBezierPath*)NIBezierPathForSlabView:(NIAnnotatedGeneratorRequestView*)view {
    return [self NIBezierPathForSlabView:view complete:NO];
}

- (NIBezierPath*)NIBezierPathForSlabView:(NIAnnotatedGeneratorRequestView*)view complete:(BOOL)complete {
    NIObliqueSliceGeneratorRequest* req = view.presentedGeneratorRequest;
    NIAffineTransform dicomToSliceTransform = NIAffineTransformInvert(req.sliceToDicomTransform);
    
    NIBezierPath* path = [self.NIBezierPath bezierPathByApplyingTransform:dicomToSliceTransform];
    
    return [self.class bezierPath:path minmax:CGFloatMax(req.slabWidth/2, view.maximumDistanceToPlane) complete:complete];
}

- (BOOL)isSolid {
    return NO;
}

- (void)drawInView:(NIAnnotatedGeneratorRequestView*)view cache:(NSMutableDictionary*)cache {
    NIObliqueSliceGeneratorRequest* req = view.presentedGeneratorRequest;
    NIAffineTransform dicomToSliceTransform = NIAffineTransformInvert(req.sliceToDicomTransform);
    
    NIBezierPath* slicePath = [self.NIBezierPath bezierPathByApplyingTransform:dicomToSliceTransform];
    
    NSColor* color = self.color;
    if ([view.highlightedAnnotations containsObject:self])
        color = [view.highlightColor colorWithAlphaComponent:color.alphaComponent];

    [[color colorWithAlphaComponent:color.alphaComponent*view.annotationsBaseAlpha] set];
    
    [slicePath.NSBezierPath stroke];
    
    // clip and draw the part in the current slab
    
    NIBezierPath *cpath = [self NIBezierPathForSlabView:view];
    
    [color set];
    [cpath.NSBezierPath stroke];
    // TODO: draw ext path with alpha, stop drawing full path
    
    // points
    
    const CGFloat radius = 0.5;
    for (NSValue* pv in [slicePath intersectionsWithPlane:NIPlaneZZero]) { // TODO: maybe only draw these where the corresponding bezier line segment has distance(bp,ep) < 1 to avoid drawing twice (especially for when we'll have opacity)
        NIVector p = pv.NIVectorValue;
        [[NSBezierPath bezierPathWithOvalInRect:NSMakeRect(p.x-radius, p.y-radius, radius*2, radius*2)] fill];
    }
    
//    return [slicePath NSBezierPath];
}

- (CGFloat)distanceToSlicePoint:(NSPoint)point cache:(NSMutableDictionary*)cache view:(NIAnnotatedGeneratorRequestView*)view closestPoint:(NSPoint*)rpoint {
    NIBezierPath* slicePath = [[self.NIBezierPath bezierPathByApplyingTransform:NIAffineTransformInvert(view.presentedGeneratorRequest.sliceToDicomTransform)] bezierPathByCollapsingZ];
    
    if (self.isSolid && [slicePath.NSBezierPath containsPoint:point]) {
        if (rpoint) *rpoint = point;
        return 0;
    }
    
    NIVector closestVector = NIVectorZero;
    [slicePath relativePositionClosestToLine:NILineMake(NIVectorMakeFromNSPoint(point), NIVectorZBasis) closestVector:&closestVector];
    
    if (rpoint) *rpoint = NSPointFromNIVector(closestVector);
    return NIVectorDistance(NIVectorMakeFromNSPoint(point), closestVector);
}

- (BOOL)intersectsSliceRect:(NSRect)rect cache:(NSMutableDictionary*)cache view:(NIAnnotatedGeneratorRequestView*)view {
    NIBezierPath* slicePath = [self.NIBezierPath bezierPathByApplyingTransform:NIAffineTransformInvert(view.presentedGeneratorRequest.sliceToDicomTransform)];
    return [slicePath.NSBezierPath intersectsRect:rect];
}

@end

@implementation NINSBezierPathAnnotation

@synthesize modelToDicomTransform = _modelToDicomTransform;

- (instancetype)initWithTransform:(NIAffineTransform)sliceToDicomTransform {
    if ((self = [super init])) {
        self.modelToDicomTransform = sliceToDicomTransform;
    }
    
    return self;
}

- (void)translate:(NIVector)translation {
    self.modelToDicomTransform = NIAffineTransformConcat(self.modelToDicomTransform, NIAffineTransformMakeTranslationWithVector(translation));
}

- (NIBezierPath*)NIBezierPath {
    if (!NIAffineTransformIsAffine(self.modelToDicomTransform))
        return nil;
    return [[NIBezierPath bezierPathWithNSBezierPath:self.NSBezierPath] bezierPathByApplyingTransform:self.modelToDicomTransform];
}

+ (NSSet*)keyPathsForValuesAffectingNIBezierPath {
    return [NSSet setWithObjects: @"NSBezierPath", @"modelToDicomTransform", nil];
}

- (NSBezierPath*)NSBezierPath {
    [NSException raise:NSInvalidArgumentException format:@"Method -[%@ NSBezierPath] must be implemented for all NINSBezierPathAnnotation subclasses", self.className];
    return nil;
}



@end