/* 
 * Copyright (C) 1995,1999 Free Software Foundation, Inc.
 * 
 * Written by:  Adam Fedor <fedor@boulder.colorado.edu>
 * Date: 1995
 * 
 * This file is part of the GNUstep Base Library.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA. */ 

#ifndef __NSRange_h_GNUSTEP_BASE_INCLUDE
#define __NSRange_h_GNUSTEP_BASE_INCLUDE

/**** Included Headers *******************************************************/

#include <Foundation/NSObject.h>

@class NSException;
@class NXConstantString;

/**** Type, Constant, and Macro Definitions **********************************/

#ifndef MAX
#define MAX(a,b) \
       ({typeof(a) _MAX_a = (a); typeof(b) _MAX_b = (b);  \
         _MAX_a > _MAX_b ? _MAX_a : _MAX_b; })
#define	GS_DEFINED_MAX
#endif

#ifndef MIN
#define MIN(a,b) \
       ({typeof(a) _MIN_a = (a); typeof(b) _MIN_b = (b);  \
         _MIN_a < _MIN_b ? _MIN_a : _MIN_b; })
#define	GS_DEFINED_MIN
#endif

/**
 * <p>
 *   The NSRange type is used to specify ranges of locations,
 *   typically items in an array, characters in a string, and bytes
 *   in a data object.
 * </p>
 * <p>
 *   As 'boundary' or 'fencepost' errors are a particularly common
 *   problem in programming, it is important that you understand
 *   how an NSRange works.
 * </p>
 * <p>
 *   An NSRange consists of a location and a length.  The points
 *   that are considered to lie in a range are the integers from
 *   the location to the location plus the length, so the number
 *   of points in a range is the length of the range plus one.<br />
 *   However, if you consider these points like the marks on a
 *   ruler, you can only store information <strong>between</strong>
 *   points.  So the number of items that can be stored in a range
 *   is the length of the range.
 * </p>
 */
typedef struct _NSRange NSRange;
struct _NSRange
{
  unsigned int location;
  unsigned int length;
};

/**** Function Prototypes ****************************************************/

/*
 *      All but the most complex functions are declared static inline in this
 *      header file so that they are maximally efficient.  In order to provide
 *      true functions (for code modules that don't have this header) this
 *      header is included in NSRange.m where the functions are no longer
 *      declared inline.
 */
#ifdef  IN_NSRANGE_M
#define GS_RANGE_SCOPE   extern
#define GS_RANGE_ATTR
#else
#define GS_RANGE_SCOPE   static inline
#define GS_RANGE_ATTR    __attribute__((unused))
#endif

GS_RANGE_SCOPE unsigned
NSMaxRange(NSRange range) GS_RANGE_ATTR;

GS_RANGE_SCOPE unsigned
NSMaxRange(NSRange range) 
{
  return range.location + range.length;
}

GS_RANGE_SCOPE BOOL 
NSLocationInRange(unsigned location, NSRange range) GS_RANGE_ATTR;

GS_RANGE_SCOPE BOOL 
NSLocationInRange(unsigned location, NSRange range) 
{
  return (location >= range.location) && (location < NSMaxRange(range));
}

GS_RANGE_SCOPE NSRange
NSMakeRange(unsigned int location, unsigned int length) GS_RANGE_ATTR;

GS_EXPORT void _NSRangeExceptionRaise (void);
/* NB: The implementation of _NSRangeExceptionRaise is: 
   [NSException raise: NSRangeException
	       format: @"Range location + length too great"]; 

   _NSRangeExceptionRaise is defined in NSRange.m so that this
   file (NSRange.h) can be included without problems in the
   implementation of the base classes themselves. */

GS_RANGE_SCOPE NSRange
NSMakeRange(unsigned int location, unsigned int length)
{
  NSRange range;
  unsigned int end = location + length;

  if (end < location || end < length)
    {
      _NSRangeExceptionRaise ();
    }
  range.location = location;
  range.length   = length;
  return range;
}

GS_RANGE_SCOPE BOOL
NSEqualRanges(NSRange range1, NSRange range2) GS_RANGE_ATTR;

GS_RANGE_SCOPE BOOL
NSEqualRanges(NSRange range1, NSRange range2)
{
  return ((range1.location == range2.location)
                && (range1.length == range2.length));
}

GS_RANGE_SCOPE NSRange
NSUnionRange(NSRange range1, NSRange range2) GS_RANGE_ATTR;

GS_RANGE_SCOPE NSRange
NSUnionRange(NSRange aRange, NSRange bRange)
{
  NSRange range;

  range.location = MIN(aRange.location, bRange.location);
  range.length   = MAX(NSMaxRange(aRange), NSMaxRange(bRange))
                - range.location;
  return range;
}

GS_RANGE_SCOPE NSRange
NSIntersectionRange(NSRange range1, NSRange range2) GS_RANGE_ATTR;

GS_RANGE_SCOPE NSRange
NSIntersectionRange (NSRange aRange, NSRange bRange)
{
  NSRange range;

  if (NSMaxRange(aRange) < bRange.location
                || NSMaxRange(bRange) < aRange.location)
    return NSMakeRange(0, 0);

  range.location = MAX(aRange.location, bRange.location);
  range.length   = MIN(NSMaxRange(aRange), NSMaxRange(bRange))
                - range.location;
  return range;
}


@class NSString;

GS_EXPORT NSString *NSStringFromRange(NSRange range);
GS_EXPORT NSRange NSRangeFromString(NSString *aString);

#ifdef	GS_DEFINED_MAX
#undef	GS_DEFINED_MAX
#undef	MAX
#endif

#ifdef	GS_DEFINED_MIN
#undef	GS_DEFINED_MIN
#undef	MIN
#endif

#ifndef	NO_GNUSTEP
/*
 * To be used inside a method for making sure that a range does not specify
 * anything outsize the size of an array/string.
 */
#define GS_RANGE_CHECK(RANGE, SIZE) \
  if (RANGE.location > SIZE || RANGE.length > (SIZE - RANGE.location)) \
    [NSException raise: NSRangeException \
                 format: @"in %s, range { %u, %u } extends beyond size (%u)", \
		 GSNameFromSelector(_cmd), RANGE.location, RANGE.length, SIZE]
#define CHECK_INDEX_RANGE_ERROR(INDEX, OVER) \
if (INDEX >= OVER) \
  [NSException raise: NSRangeException \
               format: @"in %s, index %d is out of range", \
               GSNameFromSelector(_cmd), INDEX]
#endif

#endif /* __NSRange_h_GNUSTEP_BASE_INCLUDE */