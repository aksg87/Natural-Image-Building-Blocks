//
//  NIMPRAnnotation.h
//  NIMPR
//
//  Created by Alessandro Volz on 7/8/15.
//  Copyright (c) 2015 volz.io. All rights reserved.
//

#import "NIAnnotatedGeneratorRequestView.h"

// NIAnnotationRequestCache dictionary keys
extern NSString* const NIAnnotationProjection; // NSImage
extern NSString* const NIAnnotationProjectionMask; // NSImage

extern NSString* const NIAnnotationChangeNotification; // to observe an annotation's changes you can either observe these notifications or observe the annotation's changed property
extern NSString* const NIAnnotationChangeNotificationChangesKey;

extern CGFloat const NIAnnotationDistant;

@interface NIAnnotation : NSObject <NSCoding> {
    NSString* _name;
    NSColor* _color;
    BOOL _locked;
//    NSDictionary* _userInfo;
    NSMutableDictionary* _changes;
}

+ (BOOL)isAbstract;

@property(retain) NSString* name;
+ (NSString*)name:(NIAnnotation*)annotation;
@property(retain) NSColor* color;
+ (NSColor*)color:(NIAnnotation*)annotation;
@property BOOL locked;

+ (BOOL)lockedDefault; // default is NO, some subclasses may want YES (like NIMaskAnnotation)

- (instancetype)init NS_DESIGNATED_INITIALIZER;
- (instancetype)initWithCoder:(NSCoder*)aDecoder NS_DESIGNATED_INITIALIZER;
- (void)initNIAnnotation NS_REQUIRES_SUPER;

- (void)encodeWithCoder:(NSCoder*)coder NS_REQUIRES_SUPER;

+ (NSColor*)defaultColor;
+ (void)setDefaultColor:(NSColor*)color;
- (NSColor*)color;

@property(readonly) BOOL annotation; // the value of this property is always YES, but you can observe it in order to observe changes in the annotation's properties
+ (NSSet*)keyPathsForValuesAffectingAnnotation NS_REQUIRES_SUPER;

- (void)translate:(NIVector)translation;

- (void)drawInView:(NIAnnotatedGeneratorRequestView*)view cache:(NSMutableDictionary*)cache;

- (CGFloat)distanceToSlicePoint:(NSPoint)point cache:(NSMutableDictionary*)cache view:(NIAnnotatedGeneratorRequestView*)view closestPoint:(NSPoint*)rpoint; // point is on slice
- (BOOL)intersectsSliceRect:(NSRect)rect cache:(NSMutableDictionary*)cache view:(NIAnnotatedGeneratorRequestView*)view;

- (NSSet*)handlesInView:(NIAnnotatedGeneratorRequestView*)view;

@end

extern NSString* const NIAnnotationTransformKey;

@protocol NITransformAnnotation <NSObject>

- (NIAffineTransform)modelToDicomTransform;

@end
