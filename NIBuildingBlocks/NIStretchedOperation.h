//  Copyright (c) 2017 Spaltenstein Natural Image
//  Copyright (c) 2017 OsiriX Foundation
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

#ifndef _NISTRETCHEDOPERATION_H_
#define _NISTRETCHEDOPERATION_H_

#import <Cocoa/Cocoa.h>
#import "NIGeneratorOperation.h"
#import "NIGeneratorRequest.h"

@interface NIStretchedOperation : NIGeneratorOperation
{
    volatile int32_t _outstandingFillOperationCount __attribute__ ((aligned (4)));
    
    float *_floatBytes;
    NSMutableSet *_fillOperations;
	NSOperation *_projectionOperation;
    BOOL _operationExecuting;
    BOOL _operationFinished;
    BOOL _operationFailed;
    
    CGFloat _sampleSpacing;
}

- (id)initWithRequest:(NIStretchedGeneratorRequest *)request volumeData:(NIVolumeData *)volumeData;

@property (readonly) NIStretchedGeneratorRequest *request;

@end

#endif /* _NISTRETCHEDOPERATION_H_ */
