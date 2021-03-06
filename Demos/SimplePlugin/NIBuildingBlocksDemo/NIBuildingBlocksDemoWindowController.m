//  Created by Joël Spaltenstein on 6/15/15.
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

#import <NIBuildingBlocks/NIBuildingBlocks.h>

#import "NIBuildingBlocksDemoWindowController.h"
#import "NIBuildingBlocksDemoView.h"

@interface NIBuildingBlocksDemoWindowController ()
- (void)updateVolumes;
@end

@implementation NIBuildingBlocksDemoWindowController

@synthesize volumeData =_volumeData;
@synthesize leftView =_leftView;
@synthesize rightView = _rightView;

- (void)windowDidLoad {
    [super windowDidLoad];

    _leftView.rimColor = [[NSColor greenColor] colorWithAlphaComponent:0.8];
    _leftView.displayScaleBar = YES;
    _leftView.displayOrientationLabels = YES;
    _rightView.rimColor = [[NSColor redColor] colorWithAlphaComponent:0.8];
    _rightView.displayScaleBar = YES;
    _rightView.displayOrientationLabels = YES;

    NIIntersection *leftIntersection = [[[NIIntersection alloc] init] autorelease];
    leftIntersection.color = [[NSColor redColor] colorWithAlphaComponent:0.8];
    leftIntersection.thickness = 2;
    leftIntersection.maskAroundMouse = YES;
    [leftIntersection bind:@"intersectingObject" toObject:_rightView withKeyPath:@"presentedGeneratorRequest" options:nil];
    [_leftView addIntersection:leftIntersection forKey:@"intersection"];

    NIIntersection *rightIntersection = [[[NIIntersection alloc] init] autorelease];
    rightIntersection.color = [[NSColor greenColor] colorWithAlphaComponent:0.8];
    rightIntersection.thickness = 2;
    rightIntersection.maskAroundMouse = YES;
    [rightIntersection bind:@"intersectingObject" toObject:_leftView withKeyPath:@"presentedGeneratorRequest" options:nil];
    [_rightView addIntersection:rightIntersection forKey:@"intersection"];

    [self updateVolumes];
}

- (void)dealloc
{
    [_volumeData release];
    _volumeData = nil;

    [[_leftView intersectionForKey:@"intersection"] unbind:@"intersectingObject"];
    [[_rightView intersectionForKey:@"intersection"] unbind:@"intersectingObject"];

    [super dealloc];
}

- (void)setVolumeData:(NIVolumeData *)volumeData
{
    if (_volumeData != volumeData) {
        [_volumeData release];
        _volumeData = [volumeData retain];

        [CATransaction begin];
        [CATransaction setDisableActions:YES];
        if (_leftView.volumeDataCount) {
            [_leftView removeVolumeDataAtIndex:0];
        }
        if (_rightView.volumeDataCount) {
            [_rightView removeVolumeDataAtIndex:0];
        }
        [CATransaction commit];

        [self updateVolumes];
    }
}

- (void)updateVolumes
{
    if (_volumeData) {
        [CATransaction begin];
        [CATransaction setDisableActions:YES];

        [_leftView addVolumeData:_volumeData];

        NIVolumeDataProperties *properties = [_rightView addVolumeData:_volumeData];
        properties.CLUT = [[[NSGradient alloc] initWithStartingColor:[NSColor blueColor] endingColor:[NSColor redColor]] autorelease];

        NIAffineTransform inverseVolumeTransform = NIAffineTransformInvert(_volumeData.modelToVoxelTransform);
        NIVector center = NIVectorApplyTransform(NIVectorMake(round(_volumeData.pixelsWide / 2.0), round(_volumeData.pixelsHigh / 2.0), round(_volumeData.pixelsDeep / 2.0)), inverseVolumeTransform);
        NIObliqueSliceGeneratorRequest *leftRequest = [[[NIObliqueSliceGeneratorRequest alloc] initWithCenter:center pixelsWide:200 pixelsHigh:200
                                                                                                       xBasis:_volumeData.directionX yBasis:NIVectorInvert(_volumeData.directionY)] autorelease];
        leftRequest.interpolationMode = NIInterpolationModeCubic;
        NIObliqueSliceGeneratorRequest *rightRequest = [[[NIObliqueSliceGeneratorRequest alloc] initWithCenter:center pixelsWide:200 pixelsHigh:200
                                                                                                       xBasis:_volumeData.directionX yBasis:_volumeData.directionZ] autorelease];
        rightRequest.interpolationMode = NIInterpolationModeCubic;

        _leftView.generatorRequest = leftRequest;
        _rightView.generatorRequest = rightRequest;
        [CATransaction commit];
    }
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
    return @[@"WindowLevelWindowWidthIdentifier", NSToolbarFlexibleSpaceItemIdentifier];
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar
{
    return @[@"WindowLevelWindowWidthIdentifier", NSToolbarFlexibleSpaceItemIdentifier];
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)itemIdentifier willBeInsertedIntoToolbar:(BOOL)flag
{
    NSToolbarItem *toolbarItem = [[[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier] autorelease];

    if ([itemIdentifier isEqual:@"WindowLevelWindowWidthIdentifier"]) {
        NIWindowLevelWindowWidthToolbarItem *windowingToolbarItem;
        windowingToolbarItem = [[[NIWindowLevelWindowWidthToolbarItem alloc] initWithItemIdentifier:@"WindowLevelWindowWidthIdentifier"] autorelease];
        windowingToolbarItem.generatorRequestViews = @[_leftView, _rightView];
        return windowingToolbarItem;
    } else {
        // itemIdentifier referred to a toolbar item that is not
        // not provided or supported by us or cocoa
        // Returning nil will inform the toolbar
        // this kind of item is not supported
        toolbarItem = nil;
    }
    return toolbarItem;
}


@end
