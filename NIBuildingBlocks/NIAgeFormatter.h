//  Created by Joël Spaltenstein on 10/15/15.
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

#ifndef _NIAGEFORMATTER_H_
#define _NIAGEFORMATTER_H_

#import <Cocoa/Cocoa.h>

typedef NS_ENUM(NSUInteger, NIAgeFormatterStyle) {    // date and time format styles
    NIAgeFormatterBirthdayWithDate = 0, // Birthday with Age in the format "May 2, 1980 (35)"
};

// This formatter with format a birthdate into the age at a specified reference date.
@interface NIAgeFormatter : NSFormatter
{
    NIAgeFormatterStyle _ageStyle;
    NSDate *_referenceDate;
}

@property (nonatomic, readwrite, assign) NIAgeFormatterStyle ageStyle;
@property (nonatomic, readwrite, retain) NSDate *referenceDate; // will use [NSDate date] if the referenceDate is null

- (NSString *)stringFromDate:(NSDate *)date;

@end

#endif /* _NIAGEFORMATTER_H_ */
