//  Created by Joël Spaltenstein on 6/1/15.
//  Copyright (c) 2015 Spaltenstein Natural Image
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

#import "OsiriXIntegration.h"
#import "NIBBVolumeData.h"

#import <objc/runtime.h>

@interface ViewerController_NIBB : NSObject

- (NIBBVolumeData *)NIBBVolumeDataForMovieIndex:(NSUInteger)movieIndex;

// methods that are here to specify the signature, but that won't be implemented
- (NSMutableArray*)pixList:(long)i;
- (NSData*)volumeData:(long) i;

@end

@interface DCMPix_NIBB : NSObject

// methods that are here to specify the signature, but that won't be implemented
- (double)sliceInterval;
- (double)sliceThickness;
- (double)pixelSpacingX;
- (double)pixelSpacingY;
- (double)originX;
- (double)originY;
- (double)originZ;
- (long)pwidth;
- (long)pheight;
- (void)orientationDouble:(double*)c;

@end


int NIBuildingBlocksInstallOsiriXCategories()
{
    Class ViewerControllerClass = objc_getClass("ViewerController");

    if (ViewerControllerClass == NULL) {
        NSLog(@"Couldn't install the NIBuildingBlock OsiriX Integration ViewerController methods because the ViewerController class could not be found");
        return -1;
    }

    Method NIBBVolumeDataForMovieIndexMethod = class_getInstanceMethod([ViewerController_NIBB class], @selector(NIBBVolumeDataForMovieIndex:));
    if (class_getInstanceMethod(ViewerControllerClass, @selector(NIBBVolumeDataForMovieIndex:)) == NULL) {
        class_addMethod(ViewerControllerClass, @selector(NIBBVolumeDataForMovieIndex:), method_getImplementation(NIBBVolumeDataForMovieIndexMethod), method_getTypeEncoding(NIBBVolumeDataForMovieIndexMethod));
    }

    return 0;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincomplete-implementation"

@implementation ViewerController_NIBB

- (NIBBVolumeData *)NIBBVolumeDataForMovieIndex:(NSUInteger)movieIndex
{
    NSArray *pixListForMovieIndex = [self pixList:movieIndex];
    NSData *volume = [self volumeData:movieIndex];

    DCMPix_NIBB *firstPix;
    float sliceThickness;
    NIBBAffineTransform pixToDicomTransform;
    double spacingX;
    double spacingY;
    double spacingZ;
    double orientation[9];

    firstPix = [pixListForMovieIndex objectAtIndex:0];

    sliceThickness = [firstPix sliceInterval];
    if(sliceThickness == 0)
    {
        NSLog(@"slice interval = slice thickness!");
        sliceThickness = [firstPix sliceThickness];
    }

    memset(orientation, 0, sizeof(double) * 9);
    [firstPix orientationDouble:orientation];
    spacingX = firstPix.pixelSpacingX;
    spacingY = firstPix.pixelSpacingY;
    if(sliceThickness == 0) { // if the slice thickness is still 0, make it the same as the average of the spacingX and spacingY
        sliceThickness = (spacingX + spacingY)/2.0;
    }
    spacingZ = sliceThickness;

    // test to make sure that orientation is initialized, when the volume is curved or something, it doesn't make sense to talk about orientation, and
    // so the orientation is really bogus
    // the test we will do is to make sure that orientation is 3 non-degenerate vectors
    NIBBAffineTransform transform = NIBBAffineTransformIdentity;
    transform.m11 = orientation[0];
    transform.m12 = orientation[1];
    transform.m13 = orientation[2];
    transform.m21 = orientation[3];
    transform.m22 = orientation[4];
    transform.m23 = orientation[5];
    transform.m31 = orientation[6];
    transform.m32 = orientation[7];
    transform.m33 = orientation[8];
    if (NIBBAffineTransformDeterminant(transform) == 0.0) {
        memset(orientation, 0, sizeof(double)*9);
        orientation[0] = orientation[4] = orientation[8] = 1;
    }

    pixToDicomTransform = NIBBAffineTransformIdentity;
    pixToDicomTransform.m41 = firstPix.originX;
    pixToDicomTransform.m42 = firstPix.originY;
    pixToDicomTransform.m43 = firstPix.originZ;
    pixToDicomTransform.m11 = orientation[0]*spacingX;
    pixToDicomTransform.m12 = orientation[1]*spacingX;
    pixToDicomTransform.m13 = orientation[2]*spacingX;
    pixToDicomTransform.m21 = orientation[3]*spacingY;
    pixToDicomTransform.m22 = orientation[4]*spacingY;
    pixToDicomTransform.m23 = orientation[5]*spacingY;
    pixToDicomTransform.m31 = orientation[6]*spacingZ;
    pixToDicomTransform.m32 = orientation[7]*spacingZ;
    pixToDicomTransform.m33 = orientation[8]*spacingZ;

    return [[[NIBBVolumeData alloc] initWithData:volume pixelsWide:[firstPix pwidth] pixelsHigh:[firstPix pheight] pixelsDeep:[pixListForMovieIndex count]
                                volumeTransform:NIBBAffineTransformInvert(pixToDicomTransform) outOfBoundsValue:-1000] autorelease];
}

@end
#pragma clang diagnostic pop

