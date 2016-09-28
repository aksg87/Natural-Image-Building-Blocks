//  Copyright (c) 2016 OsiriX Foundation
//  Copyright (c) 2016 Spaltenstein Natural Image
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

#import <Cocoa/Cocoa.h>
#import <Accelerate/Accelerate.h>
#import "NIGeometry.h"

NS_ASSUME_NONNULL_BEGIN

CF_EXTERN_C_BEGIN


/** Interpolation modes used by the NIGenerator
 @see NIGenerator
 @see NIVolumeData
 */
typedef NS_ENUM(NSInteger, NIInterpolationMode) {
    /** Interpolate using linear interpolation */
    NIInterpolationModeLinear,
    /** Interpolate using nearest neighbor interpolation */
    NIInterpolationModeNearestNeighbor,
    /** Interpolate using cubic interpolation */
    NIInterpolationModeCubic,

    /** Use the default interpolation mode */
    NIInterpolationModeNone = 0xFFFFFF,
};

/**
 NIVolumeDataInlineBuffer is used to make sure that values that will be often used while sampling an NIVolumeData are on the stack. The goal is to optimize CPU cache performance.
 An NIVolumeDataInlineBuffer should be used as a stack variable and then initialized using -[NIVolumeData acquireInlineBuffer:]. The NIVolumeDataInlineBuffer can then be used with
 a number of inline functions defined in NIVolumeData.h.
 @see NIVolumeData
 @see [NIVolumeData acquireInlineBuffer:]
*/
typedef struct { // build one of these on the stack and then use -[NIVolumeData acquireInlineBuffer:] to initialize it.
    const float *floatBytes;

    float outOfBoundsValue;

    NSUInteger pixelsWide;
    NSUInteger pixelsHigh;
    NSUInteger pixelsDeep;

    NIAffineTransform modelToVoxelTransform;
} NIVolumeDataInlineBuffer;

/**
 The NIVolumeData class represents a volume of float intensity data in the three natural dimensions. In addition to the floats,
 NIVolumeData includes an NIAffineTransform referred to as the modelToVoxelTransform which is used to position the volume of
 floats in the model space (DICOM space).
 
 NIVolumeData can also represent curved volumes (for example, volumes generated by NIStraightenedGeneratorRequest or NIStretchedGeneratorRequest).
 If the NIVolumeData represents a curved volume, the curved property will be true, and convertVolumeVectorToModelVector and convertVolumeVectorToModelVector
 will use the coresponding blocks to convert the points. Curved volumes can not be used as source volumes by the NIGenerator.
 
 @see NIGenerator
 @see NIMask
 */
@interface NIVolumeData : NSObject <NSCopying, NSSecureCoding> {
    NSData *_floatData;
    float _outOfBoundsValue;

    NSUInteger _pixelsWide;
    NSUInteger _pixelsHigh;
    NSUInteger _pixelsDeep;

    NIAffineTransform _modelToVoxelTransform; // modelToVoxelTransform is the transform from Model (patient) space to pixel data

    BOOL _curved;
    NIVector (^_convertVolumeVectorToModelVectorBlock)(NIVector);
    NIVector (^_convertVolumeVectorFromModelVectorBlock)(NIVector);
}

/**
 This is a utility method to help build an NIAffineTransform that places a volume in space
 @param origin The location of the origin (voxel at coordinate 0,0,0) in model space (DICOM space)
 @param directionX The direction vector of increments in voxel x coordinates, in model space (DICOM space).
 @param pixelSpacingX The pixel spacing in model space (DICOM space) in mm/pixel in the x direction.
 @param directionY The direction vector of increments in voxel y coordinates, in model space (DICOM space).
 @param pixelSpacingY The pixel spacing in model space (DICOM space) in mm/pixel in the y direction.
 @param directionZ The direction vector of increments in voxel z coordinates, in model space (DICOM space).
 @param pixelSpacingZ The pixel spacing in model space (DICOM space) in mm/pixel in the z direction.
 @return Returns an NIAffineTransform that can be used when calling initWithData:pixelsWide:pixelsHigh:pixelsDeep:modelToVoxelTransform:outOfBoundsValue:
 */
+ (NIAffineTransform)modelToVoxelTransformForOrigin:(NIVector)origin directionX:(NIVector)directionX pixelSpacingX:(CGFloat)pixelSpacingX directionY:(NIVector)directionY pixelSpacingY:(CGFloat)pixelSpacingY
                                     directionZ:(NIVector)directionZ pixelSpacingZ:(CGFloat)pixelSpacingZ;

- (instancetype)init NS_UNAVAILABLE;

- (instancetype)initWithBytesNoCopy:(const float *)floatBytes pixelsWide:(NSUInteger)pixelsWide pixelsHigh:(NSUInteger)pixelsHigh pixelsDeep:(NSUInteger)pixelsDeep
              modelToVoxelTransform:(NIAffineTransform)modelToVoxelTransform outOfBoundsValue:(float)outOfBoundsValue freeWhenDone:(BOOL)freeWhenDone __deprecated; // modelToVoxelTransform is the transform from Model (patient) space to pixel data

/**
 Returns an NIVolumeData object initialized with the given parameters

 @param data NSData object that contains the packed float intensities that the returned NIVolumeData represents.
 @param pixelsWide The width of the volume in pixels. This value must be greater than 0.
 @param pixelsHigh The height of the volume in pixels. This value must be greater than 0.
 @param pixelsDeep The depth of the volume in pixels. This value must be greater than 0.
 @param modelToVoxelTransform The NIAffineTransform that represents the mapping of coordinates from model space (DICOM space) to voxel coordinates.
 @param outOfBoundsValue The value that will be filled in by the NIGenerator when sampling pixels that are outside of the volume.
 */
- (instancetype)initWithData:(NSData *)data pixelsWide:(NSUInteger)pixelsWide pixelsHigh:(NSUInteger)pixelsHigh pixelsDeep:(NSUInteger)pixelsDeep
       modelToVoxelTransform:(NIAffineTransform)modelToVoxelTransform outOfBoundsValue:(float)outOfBoundsValue NS_DESIGNATED_INITIALIZER; // modelToVoxelTransform is the transform from Model (patient) space to pixel data

/**
 Returns a curved NIVolumeData object initialized with the given parameters

 @param data NSData object that contains the packed float intensities that the returned NIVolumeData represents.
 @param pixelsWide The width of the volume in pixels. This value must be greater than 0.
 @param pixelsHigh The height of the volume in pixels. This value must be greater than 0.
 @param pixelsDeep The depth of the volume in pixels. This value must be greater than 0.
 @param volumeToModelConverter The block that converts voxel coordinates to model space (DICOM space).
 @param modelToVolumeConverter The block that converts model space (DICOM space) to voxel coordinates.
 @param outOfBoundsValue The value that will be filled in by the NIGenerator when sampling pixels that are outside of the volume.
 @see curved
*/
 - (instancetype)initWithData:(NSData *)data pixelsWide:(NSUInteger)pixelsWide pixelsHigh:(NSUInteger)pixelsHigh pixelsDeep:(NSUInteger)pixelsDeep
      volumeToModelConverter:(NIVector (^)(NIVector volumeVector))volumeToModelConverter modelToVolumeConverter:(NIVector (^)(NIVector modelVector))modelToVolumeConverter
            outOfBoundsValue:(float)outOfBoundsValue NS_DESIGNATED_INITIALIZER;

/**
 Returns an NIVolumeData object initialized by the values from another given NIVolumeData. The floatData is copied by reference.
 
 @param volumeData The NIVolumeData from which to copy values. This value must not be nil.
*/
- (instancetype)initWithVolumeData:(NIVolumeData *)volumeData;

- (nullable instancetype)initWithCoder:(NSCoder *)decoder NS_DESIGNATED_INITIALIZER;

/**
 The width of the volume in pixels
*/
@property (readonly) NSUInteger pixelsWide;
/**
 The height of the volume in pixels
 */
@property (readonly) NSUInteger pixelsHigh;
/**
 The depth of the volume in pixels
 */
@property (readonly) NSUInteger pixelsDeep;

/**
 A Boolean value indicating whether the volume is not curved and the modelToVoxelTransform only includes scale and translation.
 */
@property (readonly, getter=isRectilinear) BOOL rectilinear;

/**
 The smallest pixel spacing, in mm/pixel, in any of the x, y and z directions.
 @see pixelSpacingX
 @see pixelSpacingY
 @see pixelSpacingZ
 */
@property (readonly) CGFloat minPixelSpacing; // the smallest pixel spacing in any direction;
/**
 The pixel spacing in model space (DICOM space) in mm/pixel in the x direction.
 */
@property (readonly) CGFloat pixelSpacingX;
/**
 The pixel spacing in model space (DICOM space) in mm/pixel in the y direction.
 */
@property (readonly) CGFloat pixelSpacingY;
/**
 The pixel spacing in model space (DICOM space) in mm/pixel in the z direction.
 */
@property (readonly) CGFloat pixelSpacingZ;

/**
 The origin The orgin of the volume (voxel coordinate (0,0,0)) in model space (DICOM space).
 */
@property (readonly) NIVector origin;
/**
 The center of the volume in model space (DICOM space).
 */
@property (readonly) NIVector center;
/**
 The direction vector of increments in voxel x coordinates, in model space (DICOM space).
 */
@property (readonly) NIVector directionX;
/**
 The direction vector of increments in voxel y coordinates, in model space (DICOM space).
 */
@property (readonly) NIVector directionY;
/**
 The direction vector of increments in voxel z coordinates, in model space (DICOM space).
 */
@property (readonly) NIVector directionZ;

/**
 The value that will be filled in by the NIGenerator when sampling pixels that are outside of the volume.
 */
@property (readonly) float outOfBoundsValue;

/**
 The NIAffineTransform that represents the mapping of coordinates from model space (DICOM space) to voxel coordinates.
*/
@property (readonly) NIAffineTransform modelToVoxelTransform; // modelToVoxelTransform is the transform from model (patient) space to pixel data

/**
 A Boolean value indicating whether the NIVolumeData is curved. If the NIVolumeData is curved the modelToVoxelTransform is irrelevant
 and instead convertVolumeVectorToModelVectorBlock and convertVolumeVectorFromModelVectorBlock are used to convert coordinates.
 Curved NIVolumeData objects can not be used as the source NIVolumeData when using the NIGenerator.
 
 @see convertVolumeVectorToModelVectorBlock
 @see convertVolumeVectorFromModelVectorBlock
 @see convertVolumeVectorToModelVector:
 @see convertVolumeVectorFromModelVector:
*/
@property (readonly, getter = isCurved) BOOL curved; // if the volume is curved the modelToVoxelTransform will be bogus, but the following properties will still work
/**
 The block that converts voxel coordinates to model space (DICOM space) for curved NIVolumeData objects.
 @see curved
 @see convertVolumeVectorFromModelVectorBlock
 @see convertVolumeVectorToModelVector:
 @see convertVolumeVectorFromModelVector:
*/
@property (nullable, readonly, copy) NIVector (^convertVolumeVectorToModelVectorBlock)(NIVector);
/**
 The block that converts model space (DICOM space) to voxel coordinates for curved NIVolumeData objects.
 @see curved
 @see convertVolumeVectorToModelVectorBlock
 @see convertVolumeVectorToModelVector:
 @see convertVolumeVectorFromModelVector:
 */
@property (nullable, readonly, copy) NIVector (^convertVolumeVectorFromModelVectorBlock)(NIVector);

/**
 Converts voxel coordinates to model space (DICOM space) coordinates.
 @param vector The voxel coordinate to transform.
 @return The point in model space (DICOM space).
 */
- (NIVector)convertVolumeVectorToModelVector:(NIVector)vector;
/**
 Converts model space (DICOM space) coordinates to voxel coordinates.
 @param vector The point in model space (DICOM space) to transform.
 @return The coordinate in voxel space.
 */
- (NIVector)convertVolumeVectorFromModelVector:(NIVector)vector;
/**
 The float intensities represented by the NIVolumeData object.
 */
@property (readonly, retain) NSData *floatData;

/**
 Will copy a row of float values in the x direction starting of the given voxel coordinate into the given buffer.
 @param buffer the buffer into which to copy the floats.
 @param x The x coordinate of the origin voxel.
 @param y The y coordinate of the origin voxel.
 @param z The z coordinate of the origin voxel.
 @param length The number of floats to copy, if the volume is not wide enough to include the given number of floats, only the floats to the end of the volume will be copied.
 @return The number of floats that were copied.
*/
- (NSUInteger)getFloatRun:(float *)buffer atPixelCoordinateX:(NSUInteger)x y:(NSUInteger)y z:(NSUInteger)z length:(NSUInteger)length;

/**
 Returns a vImage_Buffer that corresponds voxels at the given z depth.
 @param z The depth index of the 2D slice represented in the returned vImage_Buffer.
 @return A vImage_Buffer the represents the 2D slice of the given depth index.
*/
- (vImage_Buffer)floatBufferForSliceAtIndex:(NSUInteger)z;
/**
 Returns an NIVolumeData object with depth of 1 that corresponds voxels at the given z depth.
 @param z The depth index of the 2D slice represented in the returned NIVolumeData.
 @return An NIVolumeData with depth of 1 the represents the 2D slice of the given depth index.
*/
- (NIVolumeData *)volumeDataForSliceAtIndex:(NSUInteger)z;
/**
 Returns an NIVolumeData object with the subvolume described by a set of ranges within the receiver.
 @param x An NSRange that corresponds to the x indexes of the voxels to be used for the returned NIVolumeData.
 @param y An NSRange that corresponds to the y indexes of the voxels to be used for the returned NIVolumeData.
 @param z An NSRange that corresponds to the z indexes of the voxels to be used for the returned NIVolumeData.
 @return An NIVolumeData object build with the given subranges.
*/
- (NIVolumeData *)volumeDataWithIndexRangesX:(NSRange)x y:(NSRange)y z:(NSRange)z;
/**
 Returns an NIVolumeData object with the same underlying float data, but with the given in modelToVoxelTransform.
 @param modelToVoxelTransform The modelToVoxelTransform of the returned NIVolumeTransform.
 @return An NIVolumeData object with the given modelToVoxelTransform.
*/
- (instancetype)volumeDataWithModelToVoxelTransform:(NIAffineTransform)modelToVoxelTransform;

/**
 Resamples the receiver to build a new NIVolumeData object that has the orientation and pixel spacing that are described by the given modelToVoxelTransform. Given the origin and
 orientation, a minimum volume that will fit the reciever is calculated. As such, the modelToVoxelTransform of the returned NIVolumeData may not be equal to the given modelToVoxelTransform.
 However, the orientation will be conserved, and any shift is guaranteed to be a multiple of the basis vectors of the given modelToVoxelTransform.
 
 This method does not work on curved NIVolumeTransform objects.
 @param modelToVoxelTransform An NIAffineTransform that describes the orientation and pixel spacing of the desired NIVolumetransform.
 @param interpolationsMode The interpolation mode that is used when resampling the vector.
 @return Returns an NIVolumeData that represents the receiver interpolated with the given orientation and pixel spacing.
 @see NIInterpolationMode
 @see volumeDataResampledWithModelToVoxelTransform:pixelsWide:pixelsHigh:pixelsDeep:interpolationMode:
*/
- (instancetype)volumeDataResampledWithModelToVoxelTransform:(NIAffineTransform)modelToVoxelTransform interpolationMode:(NIInterpolationMode)interpolationsMode;
/**
 Resamples the receiver to produce an NIVolumeTransform with the given size and modelToVoxelTransform.
 @param modelToVoxelTransform The modelToVoxelTransform of the returned NIVolumeData.
 @param pixelsWide The width of the returned NIVolumeData.
 @param pixelsHigh The height of the returned NIVolumeData.
 @param pixelsDeep The depth of the returned NIVolumeData.
 @param interpolationsMode The interpolation mode that is used when resampling the vector.
 @return Returns an NIVolumeData that represents the receiver interpolated with the given size and modelToVoxelTransform.
 @see NIInterpolationMode
 @see volumeDataResampledWithModelToVoxelTransform:interpolationMode:
*/
- (instancetype)volumeDataResampledWithModelToVoxelTransform:(NIAffineTransform)modelToVoxelTransform pixelsWide:(NSUInteger)pixelsWide pixelsHigh:(NSUInteger)pixelsHigh pixelsDeep:(NSUInteger)pixelsDeep
                                     interpolationMode:(NIInterpolationMode)interpolationsMode;

/**
 Returns the float value of the voxel at the given coordinate in voxel space.
 @param x The x coordinate of desired voxel.
 @param y The y coordinate of desired voxel.
 @param z The z coordinate of desired voxel.
 @return The float value of the voxel.
*/
- (CGFloat)floatAtPixelCoordinateX:(NSUInteger)x y:(NSUInteger)y z:(NSUInteger)z;
/**
 Returns the float value of the point at the given coordinate in model space (DICOM space) by linear interpolation.
 Consider using an inline buffer if performance is important.
 @param vector The coordinates of the desired point in model space (DICOM space).
 @return The linearly interpolated value at the given point.
 @see acquireInlineBuffer:
*/
- (CGFloat)linearInterpolatedFloatAtModelVector:(NIVector)vector; // these are slower, use the inline buffer if you care about speed
/**
 Returns the float value of the point at the given coordinate in model space (DICOM space) by nearest neighbor interpolation.
 Consider using an inline buffer if performance is important.
 @param vector The coordinates of the desired point in model space (DICOM space).
 @return The nearest neighbor interpolation value at the given point.
 @see acquireInlineBuffer:
 */
- (CGFloat)nearestNeighborInterpolatedFloatAtModelVector:(NIVector)vector; // these are slower, use the inline buffer if you care about speed
/**
 Returns the float value of the point at the given coordinate in model space (DICOM space) by cubic interpolation.
 Consider using an inline buffer if performance is important.
 @param vector The coordinates of the desired point in model space (DICOM space).
 @return The cubic interpolated value at the given point.
 @see acquireInlineBuffer:
 */
- (CGFloat)cubicInterpolatedFloatAtModelVector:(NIVector)vector; // these are slower, use the inline buffer if you care about speed

/**
 Used to initialize an NIVolumeDataInlineBuffer that was built on the stack. The inline buffer can then be used with a number of inline functions.
 @param inlineBuffer A pointer to the NIVolumeDataInlineBuffer to be intialized.
*/
- (void)acquireInlineBuffer:(NIVolumeDataInlineBuffer *)inlineBuffer;

@end

/**
 Returns a pointer to the array of float intensities in the previously initialized NIVolumeDataInlineBuffer.
 @param inlineBuffer The inline buffer that was previously initialized using [NIVolumeData acquireInlineBuffer:]
 @see [NIVolumeData acquireInlineBuffer:]
*/
CF_INLINE const float* NIVolumeDataFloatBytes(NIVolumeDataInlineBuffer *inlineBuffer)
{
    return inlineBuffer->floatBytes;
}

/**
 Returns the value of the voxel at the given coordinate.
 @warning This function does not do any bounds checking.
 @param inlineBuffer The inline buffer that was previously initialized using [NIVolumeData acquireInlineBuffer:]
 @param x The x coordinate of the voxel.
 @param y The y coordinate of the voxel.
 @param z The z coordinate of the voxel.
 @return The value of the voxel at the given coordinate.
 @see [NIVolumeData acquireInlineBuffer:]
 */
CF_INLINE float NIVolumeDataUncheckedGetFloatAtPixelCoordinate(NIVolumeDataInlineBuffer *inlineBuffer, NSInteger x, NSInteger y, NSInteger z)
{
    return (inlineBuffer->floatBytes)[x + inlineBuffer->pixelsWide*(y + inlineBuffer->pixelsHigh*z)];
}

/**
 Returns the value of the voxel at the given coordinate.
 @param inlineBuffer The inline buffer that was previously initialized using [NIVolumeData acquireInlineBuffer:]
 @param x The x coordinate of the voxel.
 @param y The y coordinate of the voxel.
 @param z The z coordinate of the voxel.
 @return The value of the voxel at the given coordinate.
 @see [NIVolumeData acquireInlineBuffer:]
 */
CF_INLINE float NIVolumeDataGetFloatAtPixelCoordinate(NIVolumeDataInlineBuffer *inlineBuffer, NSInteger x, NSInteger y, NSInteger z)
{
    bool outside;

    if (inlineBuffer->floatBytes) {
        outside = false;

        outside |= x < 0;
        outside |= y < 0;
        outside |= z < 0;
        outside |= x >= inlineBuffer->pixelsWide;
        outside |= y >= inlineBuffer->pixelsHigh;
        outside |= z >= inlineBuffer->pixelsDeep;

        if (!outside) {
            return (inlineBuffer->floatBytes)[x + inlineBuffer->pixelsWide*(y + inlineBuffer->pixelsHigh*z)];
        } else {
            return inlineBuffer->outOfBoundsValue;
        }
    } else {
        return 0;
    }
}

/**
 Returns the index into the float intensity array for a given voxel coordinate. Returns outOfBoundsIndex if the coordinate is not in the volume.
 @param inlineBuffer The inline buffer that was previously initialized using [NIVolumeData acquireInlineBuffer:]
 @param x The x coordinate of the voxel.
 @param y The y coordinate of the voxel.
 @param z The z coordinate of the voxel.
 @param outOfBoundsIndex The index to return if the voxel coordinate is not in the volume.
 @return The index into the float intensity array for a given voxel coordinate, or outOfBoundsIndex if the coordinate is not in the volume.
*/
CF_INLINE NSInteger NIVolumeDataIndexAtCoordinate(NIVolumeDataInlineBuffer *inlineBuffer, NSInteger x, NSInteger y, NSInteger z, NSInteger outOfBoundsIndex)
{
    if (x < 0 || x >= inlineBuffer->pixelsWide ||
        y < 0 || y >= inlineBuffer->pixelsHigh ||
        z < 0 || z >= inlineBuffer->pixelsDeep) {
        return outOfBoundsIndex;
    }
    return x + inlineBuffer->pixelsWide*(y + inlineBuffer->pixelsHigh*z);
}

/**
 Returns the index into the float intensity array for a given voxel coordinate.
 @warning This function does not do any bounds checking.
 @param inlineBuffer The inline buffer that was previously initialized using [NIVolumeData acquireInlineBuffer:]
 @param x The x coordinate of the voxel.
 @param y The y coordinate of the voxel.
 @param z The z coordinate of the voxel.
 @return The index into the float intensity array for a given voxel coordinate.
*/
CF_INLINE NSInteger NIVolumeDataUncheckedIndexAtCoordinate(NIVolumeDataInlineBuffer *inlineBuffer, NSInteger x, NSInteger y, NSInteger z)
{
    return x + inlineBuffer->pixelsWide*(y + inlineBuffer->pixelsHigh*z);
}

/**
 Gets the indexes into the float intensity array for the 8 neighboring coordinates that need to be looked at for linear interpolation. The input
 coordinate is the floor of the floating point coordinate that is being looked up.
 @param inlineBuffer The inline buffer that was previously initialized using [NIVolumeData acquireInlineBuffer:]
 @param linearIndexes An array of indexes that will be filled out with the resulting indexes.
 @param x The x coordinate of the voxel.
 @param y The y coordinate of the voxel.
 @param z The z coordinate of the voxel.
*/
CF_INLINE void NIVolumeDataGetLinearIndexes(NIVolumeDataInlineBuffer *inlineBuffer, NSInteger linearIndexes[8], NSInteger x, NSInteger y, NSInteger z, NSInteger outOfBoundsIndex)
{
    if (x < 0 || y < 0 || z < 0 || x >= inlineBuffer->pixelsWide-1 || y >= inlineBuffer->pixelsHigh-1 || z >= inlineBuffer->pixelsDeep-1) {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    linearIndexes[i+2*(j+2*k)] = NIVolumeDataIndexAtCoordinate(inlineBuffer, x+i, y+j, z+k, outOfBoundsIndex);
                }
            }
        }
    } else {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    linearIndexes[i+2*(j+2*k)] = NIVolumeDataUncheckedIndexAtCoordinate(inlineBuffer, x+i, y+j, z+k);
                }
            }
        }
    }
}

/**
 Returns the linear interpolated float intensity at the given point in voxel space.
 @param inlineBuffer The inline buffer that was previously initialized using [NIVolumeData acquireInlineBuffer:]
 @param x The x coordinate of the voxel.
 @param y The y coordinate of the voxel.
 @param z The z coordinate of the voxel.
*/
CF_INLINE float NIVolumeDataLinearInterpolatedFloatAtVolumeCoordinate(NIVolumeDataInlineBuffer *inlineBuffer, CGFloat x, CGFloat y, CGFloat z) // coordinate in the pixel space
{

#if CGFLOAT_IS_DOUBLE
    const CGFloat x_floor = floor(x);
    const CGFloat y_floor = floor(y);
    const CGFloat z_floor = floor(z);
#else
    const CGFloat x_floor = floorf(x);
    const CGFloat y_floor = floorf(y);
    const CGFloat z_floor = floorf(z);
#endif

    // this is a horible hack, but it works
    // what I'm doing is looking at memory addresses to find an index into inlineBuffer->floatBytes that would jump out of
    // the array and instead point to inlineBuffer->outOfBoundsValue which is on the stack
    // This relies on both inlineBuffer->floatBytes and inlineBuffer->outOfBoundsValue being on a sizeof(float) boundry
    NSInteger outOfBoundsIndex = (((NSInteger)&(inlineBuffer->outOfBoundsValue)) - ((NSInteger)inlineBuffer->floatBytes)) / sizeof(float);

    NSInteger linearIndexes[8];
    NIVolumeDataGetLinearIndexes(inlineBuffer, linearIndexes, x_floor, y_floor, z_floor, outOfBoundsIndex);

    const float *floatBytes = inlineBuffer->floatBytes;

    const CGFloat dx1 = x-x_floor;
    const CGFloat dy1 = y-y_floor;
    const CGFloat dz1 = z-z_floor;

    const CGFloat dx0 = 1.0 - dx1;
    const CGFloat dy0 = 1.0 - dy1;
    const CGFloat dz0 = 1.0 - dz1;

    return (dz0*(dy0*(dx0*floatBytes[linearIndexes[0+2*(0+2*0)]] + dx1*floatBytes[linearIndexes[1+2*(0+2*0)]]) +
                 dy1*(dx0*floatBytes[linearIndexes[0+2*(1+2*0)]] + dx1*floatBytes[linearIndexes[1+2*(1+2*0)]]))) +
           (dz1*(dy0*(dx0*floatBytes[linearIndexes[0+2*(0+2*1)]] + dx1*floatBytes[linearIndexes[1+2*(0+2*1)]]) +
                 dy1*(dx0*floatBytes[linearIndexes[0+2*(1+2*1)]] + dx1*floatBytes[linearIndexes[1+2*(1+2*1)]])));
}

/**
 Returns the nearest neighbor interpolated float intensity at the given point in voxel space.
 @param inlineBuffer The inline buffer that was previously initialized using [NIVolumeData acquireInlineBuffer:]
 @param x The x coordinate of the voxel.
 @param y The y coordinate of the voxel.
 @param z The z coordinate of the voxel.
 */
CF_INLINE float NIVolumeDataNearestNeighborInterpolatedFloatAtVolumeCoordinate(NIVolumeDataInlineBuffer *inlineBuffer, CGFloat x, CGFloat y, CGFloat z) // coordinate in the pixel space
{
#if CGFLOAT_IS_DOUBLE
    NSInteger roundX = round(x);
    NSInteger roundY = round(y);
    NSInteger roundZ = round(z);
#else
    NSInteger roundX = roundf(x);
    NSInteger roundY = roundf(y);
    NSInteger roundZ = roundf(z);
#endif

    return NIVolumeDataGetFloatAtPixelCoordinate(inlineBuffer, roundX, roundY, roundZ);
}

/**
 Gets the indexes into the float intensity array for the 64 neighboring coordinates that need to be looked at for cubic interpolation. The input
 coordinate is the floor of the floating point coordinate that is being looked up.
 @param inlineBuffer The inline buffer that was previously initialized using [NIVolumeData acquireInlineBuffer:]
 @param cubicIndexes An array of indexes that will be filled out with the resulting indexes.
 @param x The x coordinate of the voxel.
 @param y The y coordinate of the voxel.
 @param z The z coordinate of the voxel.
*/
CF_INLINE void NIVolumeDataGetCubicIndexes(NIVolumeDataInlineBuffer *inlineBuffer, NSInteger cubicIndexes[64], NSInteger x, NSInteger y, NSInteger z, NSInteger outOfBoundsIndex)
{
    if (x <= 0 || y <= 0 || z <= 0 || x >= inlineBuffer->pixelsWide-2 || y >= inlineBuffer->pixelsHigh-2 || z >= inlineBuffer->pixelsDeep-2) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                for (int k = 0; k < 4; ++k) {
                    cubicIndexes[i+4*(j+4*k)] = NIVolumeDataIndexAtCoordinate(inlineBuffer, x+i-1, y+j-1, z+k-1, outOfBoundsIndex);
                }
            }
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                for (int k = 0; k < 4; ++k) {
                    cubicIndexes[i+4*(j+4*k)] = NIVolumeDataUncheckedIndexAtCoordinate(inlineBuffer, x+i-1, y+j-1, z+k-1);
                }
            }
        }
    }
}

/**
 Returns the cubic interpolated float intensity at the given point in voxel space.
 @param inlineBuffer The inline buffer that was previously initialized using [NIVolumeData acquireInlineBuffer:]
 @param x The x coordinate of the voxel.
 @param y The y coordinate of the voxel.
 @param z The z coordinate of the voxel.
 */
CF_INLINE float NIVolumeDataCubicInterpolatedFloatAtVolumeCoordinate(NIVolumeDataInlineBuffer *inlineBuffer, CGFloat x, CGFloat y, CGFloat z) // coordinate in the pixel space
{
#if CGFLOAT_IS_DOUBLE
    const CGFloat x_floor = floor(x);
    const CGFloat y_floor = floor(y);
    const CGFloat z_floor = floor(z);
#else
    const CGFloat x_floor = floorf(x);
    const CGFloat y_floor = floorf(y);
    const CGFloat z_floor = floorf(z);
#endif

    const CGFloat dx = x-x_floor;
    const CGFloat dy = y-y_floor;
    const CGFloat dz = z-z_floor;

    const CGFloat dxx = dx*dx;
    const CGFloat dxxx = dxx*dx;

    const CGFloat dyy = dy*dy;
    const CGFloat dyyy = dyy*dy;

    const CGFloat dzz = dz*dz;
    const CGFloat dzzz = dzz*dz;

    const CGFloat wx0 = 0.5 * (    - dx + 2.0*dxx -       dxxx);
    const CGFloat wx1 = 0.5 * (2.0      - 5.0*dxx + 3.0 * dxxx);
    const CGFloat wx2 = 0.5 * (      dx + 4.0*dxx - 3.0 * dxxx);
    const CGFloat wx3 = 0.5 * (         -     dxx +       dxxx);

    const CGFloat wy0 = 0.5 * (    - dy + 2.0*dyy -       dyyy);
    const CGFloat wy1 = 0.5 * (2.0      - 5.0*dyy + 3.0 * dyyy);
    const CGFloat wy2 = 0.5 * (      dy + 4.0*dyy - 3.0 * dyyy);
    const CGFloat wy3 = 0.5 * (         -     dyy +       dyyy);

    const CGFloat wz0 = 0.5 * (    - dz + 2.0*dzz -       dzzz);
    const CGFloat wz1 = 0.5 * (2.0      - 5.0*dzz + 3.0 * dzzz);
    const CGFloat wz2 = 0.5 * (      dz + 4.0*dzz - 3.0 * dzzz);
    const CGFloat wz3 = 0.5 * (         -     dzz +       dzzz);

    // this is a horible hack, but it works
    // what I'm doing is looking at memory addresses to find an index into inlineBuffer->floatBytes that would jump out of
    // the array and instead point to inlineBuffer->outOfBoundsValue which is on the stack
    // This relies on both inlineBuffer->floatBytes and inlineBuffer->outOfBoundsValue being on a sizeof(float) boundry
    NSInteger outOfBoundsIndex = (((NSInteger)&(inlineBuffer->outOfBoundsValue)) - ((NSInteger)inlineBuffer->floatBytes)) / sizeof(float);

    NSInteger cubicIndexes[64];
    NIVolumeDataGetCubicIndexes(inlineBuffer, cubicIndexes, x_floor, y_floor, z_floor, outOfBoundsIndex);

    const float *floatBytes = inlineBuffer->floatBytes;

    return wz0*(
                wy0*(wx0 * floatBytes[cubicIndexes[0+4*(0+4*0)]] + wx1 * floatBytes[cubicIndexes[1+4*(0+4*0)]] +  wx2 * floatBytes[cubicIndexes[2+4*(0+4*0)]] + wx3 * floatBytes[cubicIndexes[3+4*(0+4*0)]]) +
                wy1*(wx0 * floatBytes[cubicIndexes[0+4*(1+4*0)]] + wx1 * floatBytes[cubicIndexes[1+4*(1+4*0)]] +  wx2 * floatBytes[cubicIndexes[2+4*(1+4*0)]] + wx3 * floatBytes[cubicIndexes[3+4*(1+4*0)]]) +
                wy2*(wx0 * floatBytes[cubicIndexes[0+4*(2+4*0)]] + wx1 * floatBytes[cubicIndexes[1+4*(2+4*0)]] +  wx2 * floatBytes[cubicIndexes[2+4*(2+4*0)]] + wx3 * floatBytes[cubicIndexes[3+4*(2+4*0)]]) +
                wy3*(wx0 * floatBytes[cubicIndexes[0+4*(3+4*0)]] + wx1 * floatBytes[cubicIndexes[1+4*(3+4*0)]] +  wx2 * floatBytes[cubicIndexes[2+4*(3+4*0)]] + wx3 * floatBytes[cubicIndexes[3+4*(3+4*0)]])
                ) +
    wz1*(
         wy0*(wx0 * floatBytes[cubicIndexes[0+4*(0+4*1)]] + wx1 * floatBytes[cubicIndexes[1+4*(0+4*1)]] +  wx2 * floatBytes[cubicIndexes[2+4*(0+4*1)]] + wx3 * floatBytes[cubicIndexes[3+4*(0+4*1)]]) +
         wy1*(wx0 * floatBytes[cubicIndexes[0+4*(1+4*1)]] + wx1 * floatBytes[cubicIndexes[1+4*(1+4*1)]] +  wx2 * floatBytes[cubicIndexes[2+4*(1+4*1)]] + wx3 * floatBytes[cubicIndexes[3+4*(1+4*1)]]) +
         wy2*(wx0 * floatBytes[cubicIndexes[0+4*(2+4*1)]] + wx1 * floatBytes[cubicIndexes[1+4*(2+4*1)]] +  wx2 * floatBytes[cubicIndexes[2+4*(2+4*1)]] + wx3 * floatBytes[cubicIndexes[3+4*(2+4*1)]]) +
         wy3*(wx0 * floatBytes[cubicIndexes[0+4*(3+4*1)]] + wx1 * floatBytes[cubicIndexes[1+4*(3+4*1)]] +  wx2 * floatBytes[cubicIndexes[2+4*(3+4*1)]] + wx3 * floatBytes[cubicIndexes[3+4*(3+4*1)]])
         ) +
    wz2*(
         wy0*(wx0 * floatBytes[cubicIndexes[0+4*(0+4*2)]] + wx1 * floatBytes[cubicIndexes[1+4*(0+4*2)]] +  wx2 * floatBytes[cubicIndexes[2+4*(0+4*2)]] + wx3 * floatBytes[cubicIndexes[3+4*(0+4*2)]]) +
         wy1*(wx0 * floatBytes[cubicIndexes[0+4*(1+4*2)]] + wx1 * floatBytes[cubicIndexes[1+4*(1+4*2)]] +  wx2 * floatBytes[cubicIndexes[2+4*(1+4*2)]] + wx3 * floatBytes[cubicIndexes[3+4*(1+4*2)]]) +
         wy2*(wx0 * floatBytes[cubicIndexes[0+4*(2+4*2)]] + wx1 * floatBytes[cubicIndexes[1+4*(2+4*2)]] +  wx2 * floatBytes[cubicIndexes[2+4*(2+4*2)]] + wx3 * floatBytes[cubicIndexes[3+4*(2+4*2)]]) +
         wy3*(wx0 * floatBytes[cubicIndexes[0+4*(3+4*2)]] + wx1 * floatBytes[cubicIndexes[1+4*(3+4*2)]] +  wx2 * floatBytes[cubicIndexes[2+4*(3+4*2)]] + wx3 * floatBytes[cubicIndexes[3+4*(3+4*2)]])
         ) +
    wz3*(
         wy0*(wx0 * floatBytes[cubicIndexes[0+4*(0+4*3)]] + wx1 * floatBytes[cubicIndexes[1+4*(0+4*3)]] +  wx2 * floatBytes[cubicIndexes[2+4*(0+4*3)]] + wx3 * floatBytes[cubicIndexes[3+4*(0+4*3)]]) +
         wy1*(wx0 * floatBytes[cubicIndexes[0+4*(1+4*3)]] + wx1 * floatBytes[cubicIndexes[1+4*(1+4*3)]] +  wx2 * floatBytes[cubicIndexes[2+4*(1+4*3)]] + wx3 * floatBytes[cubicIndexes[3+4*(1+4*3)]]) +
         wy2*(wx0 * floatBytes[cubicIndexes[0+4*(2+4*3)]] + wx1 * floatBytes[cubicIndexes[1+4*(2+4*3)]] +  wx2 * floatBytes[cubicIndexes[2+4*(2+4*3)]] + wx3 * floatBytes[cubicIndexes[3+4*(2+4*3)]]) +
         wy3*(wx0 * floatBytes[cubicIndexes[0+4*(3+4*3)]] + wx1 * floatBytes[cubicIndexes[1+4*(3+4*3)]] +  wx2 * floatBytes[cubicIndexes[2+4*(3+4*3)]] + wx3 * floatBytes[cubicIndexes[3+4*(3+4*3)]])
         );
}

__attribute__((deprecated("convert the vector using [-[NIVolumeData convertVolumeVectorFromModelVector:] first")))
CF_INLINE float NIVolumeDataLinearInterpolatedFloatAtModelVector(NIVolumeDataInlineBuffer *inlineBuffer, NIVector vector) // coordinate in mm model space
{
    vector = NIVectorApplyTransform(vector, inlineBuffer->modelToVoxelTransform);
    return NIVolumeDataLinearInterpolatedFloatAtVolumeCoordinate(inlineBuffer, vector.x, vector.y, vector.z);
}

__attribute__((deprecated("convert the vector using [-[NIVolumeData convertVolumeVectorFromModelVector:] first")))
CF_INLINE float NIVolumeDataNearestNeighborInterpolatedFloatAtModelVector(NIVolumeDataInlineBuffer *inlineBuffer, NIVector vector) // coordinate in mm model space
{
    vector = NIVectorApplyTransform(vector, inlineBuffer->modelToVoxelTransform);
    return NIVolumeDataNearestNeighborInterpolatedFloatAtVolumeCoordinate(inlineBuffer, vector.x, vector.y, vector.z);
}

__attribute__((deprecated("convert the vector using [-[NIVolumeData convertVolumeVectorFromModelVector:] first")))
CF_INLINE float NIVolumeDataCubicInterpolatedFloatAtModelVector(NIVolumeDataInlineBuffer *inlineBuffer, NIVector vector) // coordinate in mm model space
{
    vector = NIVectorApplyTransform(vector, inlineBuffer->modelToVoxelTransform);
    return NIVolumeDataCubicInterpolatedFloatAtVolumeCoordinate(inlineBuffer, vector.x, vector.y, vector.z);
}

/**
 Returns the linear interpolated float intensity for the given point in model space (DICOM space).
 @param inlineBuffer The inline buffer that was previously initialized using [NIVolumeData acquireInlineBuffer:]
 @param vector The point in model space (DICOM space).
 */
CF_INLINE float NIVolumeDataLinearInterpolatedFloatAtVolumeVector(NIVolumeDataInlineBuffer *inlineBuffer, NIVector vector)
{
    return NIVolumeDataLinearInterpolatedFloatAtVolumeCoordinate(inlineBuffer, vector.x, vector.y, vector.z);
}

/**
 Returns the nearest neighbor interpolated float intensity for the given point in model space (DICOM space).
 @param inlineBuffer The inline buffer that was previously initialized using [NIVolumeData acquireInlineBuffer:]
 @param vector The point in model space (DICOM space).
 */
CF_INLINE float NIVolumeDataNearestNeighborInterpolatedFloatAtVolumeVector(NIVolumeDataInlineBuffer *inlineBuffer, NIVector vector)
{
    return NIVolumeDataNearestNeighborInterpolatedFloatAtVolumeCoordinate(inlineBuffer, vector.x, vector.y, vector.z);
}

/**
 Returns the cubic interpolated float intensity for the given point in model space (DICOM space).
 @param inlineBuffer The inline buffer that was previously initialized using [NIVolumeData acquireInlineBuffer:]
 @param vector The point in model space (DICOM space).
 */
CF_INLINE float NIVolumeDataCubicInterpolatedFloatAtVolumeVector(NIVolumeDataInlineBuffer *inlineBuffer, NIVector vector)
{
    return NIVolumeDataCubicInterpolatedFloatAtVolumeCoordinate(inlineBuffer, vector.x, vector.y, vector.z);
}

CF_EXTERN_C_END

NS_ASSUME_NONNULL_END
