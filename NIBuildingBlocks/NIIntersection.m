//  Created by Joël Spaltenstein on 4/26/15.
//  Copyright (c) 2017 Spaltenstein Natural Image
//  Copyright (c) 2017 Michael Hilker and Andreas Holzamer
//  Copyright (c) 2017 volz io
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#import "NIIntersection.h"
#import "NIIntersectionPrivate.h"
#import "NIGeneratorRequest.h"
#import "NIGeometry.h"
#import "NIObliqueSliceIntersectionLayer.h"
#import <QuartzCore/QuartzCore.h>

NS_ASSUME_NONNULL_BEGIN

@interface NIIntersection ()

- (void)updateLayer;

@end

@implementation NIIntersection

- (instancetype)init
{
    if ( (self = [super init]) ) {
        _color = [[NSColor whiteColor] retain];
        _thickness = 1.5;
        _maskAroundMouse = YES;
        _maskAroundMouseRadius = 80;
        _maskCirclePointRadius = 80;
        _centerBulletPointRadius = 4;
    }
    return self;
}

@synthesize intersectingObject = _intersectingObject;
@synthesize maskAroundMouse = _maskAroundMouse;
@synthesize maskAroundMouseRadius = _maskAroundMouseRadius;
@synthesize maskAroundCirclePoint = _maskAroundCirclePoint;
@synthesize maskCirclePoint = _maskCirclePoint;
@synthesize maskCirclePointRadius = _maskCirclePointRadius;
@synthesize centerBulletPoint = _centerBulletPoint;
@synthesize centerBulletPointRadius = _centerBulletPointRadius;
@synthesize color = _color;
@synthesize thickness = _thickness;
@synthesize dashingLengths = _dashingLengths;

- (void)dealloc
{
    [_intersectionLayer release];
    _intersectionLayer = nil;

    [_intersectingObject removeObserver:self forKeyPath:@"rimPath"];
    [_intersectingObject release];
    _intersectingObject = nil;

    [_dashingLengths release];
    _dashingLengths = nil;

    [_color release];
    _color = nil;

    [super dealloc];
}

- (void)setIntersectingObject:(nullable id)intersectingObject
{
    if (intersectingObject && [_intersectingObject isEqual:intersectingObject] == NO) {
        [_intersectingObject removeObserver:self forKeyPath:@"rimPath" context:&self->_intersectingObject];
        [_intersectingObject release];
        _intersectingObject = [intersectingObject retain];
        [_intersectingObject addObserver:self forKeyPath:@"rimPath" options:0 context:&self->_intersectingObject];
        [self updateLayer];
    }
}

- (void)setMaskAroundMouse:(BOOL)maskAroundMouse
{
    if (_maskAroundMouse != maskAroundMouse) {
        _maskAroundMouse = maskAroundMouse;
        [self updateLayer];
    }
}

- (void)setMaskAroundMouseRadius:(CGFloat)maskAroundMouseRadius
{
    if (_maskAroundMouseRadius != maskAroundMouseRadius) {
        _maskAroundMouseRadius = maskAroundMouseRadius;
        [self updateLayer];
    }
}

- (void)setMaskAroundCirclePoint:(BOOL)maskAroundCirclePoint
{
    if (_maskAroundCirclePoint != maskAroundCirclePoint) {
        _maskAroundCirclePoint = maskAroundCirclePoint;
        [self updateLayer];
    }
}

- (void)setMaskCirclePoint:(NSPoint)maskCirclePoint
{
    if (NSEqualPoints(_maskCirclePoint, maskCirclePoint) == NO) {
        _maskCirclePoint = maskCirclePoint;
        [self updateLayer];
    }
}

- (void)setMaskCirclePointRadius:(CGFloat)maskCirclePointRadius
{
    if (_maskCirclePointRadius != maskCirclePointRadius) {
        _maskCirclePointRadius = maskCirclePointRadius;
        [self updateLayer];
    }
}

- (void)setCenterBulletPoint:(BOOL)centerBulletPoint
{
    if (_centerBulletPoint != centerBulletPoint) {
        _centerBulletPoint = centerBulletPoint;
        [self updateLayer];
    }
}

- (void)setCenterBulletPointRadius:(CGFloat)centerBulletPointRadius
{
    if (_centerBulletPointRadius != centerBulletPointRadius) {
        _centerBulletPointRadius = centerBulletPointRadius;
        [self updateLayer];
    }
}

- (void)setColor:(NSColor *)color
{
    if (_color != color) {
        [_color release];
        _color = [color retain];
        [self updateLayer];
    }
}

- (void)setThickness:(CGFloat)thickness
{
    if (_thickness != thickness) {
        _thickness = thickness;
        [self updateLayer];
    }
}

- (void)setDashingLengths:(nullable NSArray<NSNumber *> *)dashingLengths
{
    if (_dashingLengths != dashingLengths) {
        [_dashingLengths release];
        _dashingLengths = [dashingLengths copy];
        [self updateLayer];
    }
}

- (void)updateLayer
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];

    self.intersectionLayer.intersectionColor = self.color;
    self.intersectionLayer.intersectionThickness = self.thickness;
    self.intersectionLayer.intersectionDashingLengths = self.dashingLengths;
    self.intersectionLayer.rimPath = [self.intersectingObject performSelector:@selector(rimPath)];
    self.intersectionLayer.mouseGapRadius = self.maskAroundMouseRadius;
    self.intersectionLayer.gapAroundPosition = self.maskAroundCirclePoint;
    self.intersectionLayer.gapPosition = self.maskCirclePoint;
    self.intersectionLayer.gapRadius = self.maskCirclePointRadius;

    self.intersectionLayer.centerBulletPoint = self.centerBulletPoint;
    self.intersectionLayer.centerBulletPointRadius = self.centerBulletPointRadius;

    if (_mouseInBounds && self.maskAroundMouse) {
        self.intersectionLayer.gapAroundMouse = YES;
        self.intersectionLayer.mouseGapPosition = _mousePosition;
    } else {
        self.intersectionLayer.gapAroundMouse = NO;
    }

    [CATransaction commit];
}

- (void)observeValueForKeyPath:(nullable NSString *)keyPath ofObject:(nullable id)object change:(nullable NSDictionary<NSString *,id> *)change context:(nullable void *)context
{
    if (context == &_intersectingObject) {
        [self updateLayer];
    } else {
        return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}

- (void)mouseEntered:(NSEvent *)theEvent
{
    _mouseInBounds = YES;
    NSPoint pointInView = [_generatorRequestView convertPoint:theEvent.locationInWindow fromView:nil];
    CGPoint pointInLayer = [_intersectionLayer convertPoint:NSPointToCGPoint(pointInView) fromLayer:_generatorRequestView.layer];
    _mousePosition = NSPointFromCGPoint(pointInLayer);
    if (self.maskAroundMouse) {
        [self updateLayer];
    }
}

- (void)mouseExited:(NSEvent *)theEvent
{
    _mouseInBounds = NO;
    [self updateLayer];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    NSPoint pointInView = [_generatorRequestView convertPoint:theEvent.locationInWindow fromView:nil];
    CGPoint pointInLayer = [_intersectionLayer convertPoint:NSPointToCGPoint(pointInView) fromLayer:_generatorRequestView.layer];
    _mousePosition = NSPointFromCGPoint(pointInLayer);
    if (self.maskAroundMouse) {
        [self updateLayer];
    }
}

- (CGFloat)distanceToPoint:(NSPoint)point closestPoint:(nullable NSPointPointer)rpoint
{
    if (_generatorRequestView == nil || _intersectingObject == nil || !_generatorRequestView.presentedGeneratorRequest) {;
        return CGFLOAT_MAX;
    }

    NIPlane plane = [(NIObliqueSliceGeneratorRequest *)_generatorRequestView.presentedGeneratorRequest plane];
    NSArray *intersections = [[(NIObliqueSliceGeneratorRequest *)_intersectingObject rimPath] intersectionsWithPlane:plane];
    if ([intersections count] != 2) {
        return CGFLOAT_MAX;
    }

    NIMutableBezierPath *intersectionPath = [NIMutableBezierPath bezierPath];
    [intersectionPath moveToVector:NIVectorMakeFromNSPoint([_generatorRequestView convertPointFromModelVector:[intersections[0] NIVectorValue]])];
    [intersectionPath lineToVector:NIVectorMakeFromNSPoint([_generatorRequestView convertPointFromModelVector:[intersections[1] NIVectorValue]])];

    NIVector closestPoint = [intersectionPath vectorAtRelativePosition:[intersectionPath relativePositionClosestToVector:NIVectorMakeFromNSPoint(point)]];
    
    if (rpoint)
        *rpoint = NSPointFromNIVector(closestPoint);
    return NIVectorDistance(NIVectorMakeFromNSPoint(point), closestPoint);
}

@end



@implementation NIIntersection (Private)

- (NIGeneratorRequestView *)generatorRequestView
{
    return _generatorRequestView;
}

- (void)setGeneratorRequestView:(NIGeneratorRequestView *)generatorRequestView
{
    _generatorRequestView = generatorRequestView;
}

- (CALayer<NISliceIntersectionLayer> *)intersectionLayer
{
    return _intersectionLayer;
}

- (void)setIntersectionLayer:(CALayer<NISliceIntersectionLayer> *)intersectionLayer
{
    if (_intersectionLayer != intersectionLayer) {
        [_intersectionLayer release];
        _intersectionLayer = [intersectionLayer retain];
        [self updateLayer];
    }
}

@end



NS_ASSUME_NONNULL_END






