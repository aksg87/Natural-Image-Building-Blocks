//  Created by Joël Spaltenstein on 6/4/15.
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

#import <Cocoa/Cocoa.h>

@class NIWindowingView;

@interface NIWindowLevelWindowWidthToolbarItem : NSToolbarItem
{
    NSPopover *_popover;
    NIWindowingView* _windowingView;
    BOOL _observingView; // YES if the toolbarItem is currently observing the view

    NSArray *_generatorRequestViews;
    NSArray *_volumeDataProperties;
    NSUInteger _volumeDataIndex;
}

@property (nonatomic, readwrite, retain) IBOutlet NSPopover *popover;
@property (nonatomic, readwrite, retain) IBOutlet NIWindowingView* windowingView;
@property (nonatomic, readwrite, copy) NSArray *generatorRequestViews;
@property (nonatomic, readwrite, assign) NSUInteger volumeDataIndex;

@end