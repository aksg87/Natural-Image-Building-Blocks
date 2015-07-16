//
//  NIMPRAnnotatedGeneratorRequestView.h
//  NIMPR
//
//  Created by Alessandro Volz on 7/8/15.
//  Copyright (c) 2015 volz.io. All rights reserved.
//

#import <NIBuildingBlocks/NIBuildingBlocks.h>

@class NIAnnotation;

@interface NIAnnotatedGeneratorRequestView : NIGeneratorRequestView {
    CALayer* _annotationsLayer;
    NSMutableSet* _annotations;
    NSMutableSet* _glowingAnnotations;
}

@property (readonly, retain) CALayer* annotationsLayer;

- (NSMutableSet*)publicAnnotations;
- (NSMutableSet*)publicGlowingAnnotations;

- (CGFloat)maximumDistanceToPlane;

- (NIAnnotation*)annotationClosestToPoint:(NSPoint)location closestPoint:(NSPoint*)closestPoint distance:(CGFloat*)distance;
- (NIAnnotation*)annotationClosestToPoint:(NSPoint)location closestPoint:(NSPoint*)closestPoint distance:(CGFloat*)distance filter:(BOOL (^)(NIAnnotation* annotation))filter;

@end

@interface NIAnnotatedGeneratorRequestView (Super)

// NIAnnotations currently only support NIAffineTransform-based requests
@property (nonatomic, readwrite, retain) NIObliqueSliceGeneratorRequest* generatorRequest;
@property (nonatomic, readonly, copy) NIObliqueSliceGeneratorRequest* presentedGeneratorRequest;

@end