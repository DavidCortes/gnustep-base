/* A fast (Inline) array implementation without objc method overhead.
 * Copyright (C) 1998,1999  Free Software Foundation, Inc.
 * 
 * Author:	Richard Frith-Macdonald <richard@brainstorm.co.uk>
 * Created:	Nov 1998
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

#include <Foundation/NSObject.h>
#include <Foundation/NSZone.h>

/* To easily un-inline functions for debugging */
#ifndef	INLINE
#define INLINE inline
#endif

/* To turn assertions on, define GSI_ARRAY_CHECKS */
#ifdef	GSI_ARRAY_CHECKS
#define	GSI_ARRAY_CHECK NSCAssert(array->count <= array->cap && array->old <= array->cap && array->old >= 1, NSInternalInconsistencyException)
#else
#define	GSI_ARRAY_CHECK
#endif

/*
 This file should be INCLUDED in files wanting to use the GSIArray
 *	functions - these are all declared inline for maximum performance.
 *
 *	The file including this one may predefine some macros to alter
 *	the behaviour (default macros assume the items are NSObjects
 *	that are to be retained in the array) ...
 *
 *	GSI_ARRAY_RETAIN()
 *		Macro to retain an array item
 *
 *	GSI_ARRAY_RELEASE()
 *		Macro to release the item.
 *
 *	The next two values can be defined in order to let us optimise
 *	even further when either retain or release operations are not needed.
 *
 *	GSI_ARRAY_NO_RELEASE
 *		Defined if no release operation is needed for an item
 *	GSI_ARRAY_NO_RETAIN
 *		Defined if no retain operation is needed for a an item
 *		 
 *	The value GSI_ARRAY_EXTRA may be defined as the type of an extra
 *	field produced in every array.  The name of this field is 'extra'.
 *
 *	The value GSI_ARRAY_TYPE may be defined as an additional type
 *	which must be valid as an array element.  Otherwise the types are
 *	controlled by the mechanism in GSUnion.h
 */


#ifdef	GSI_ARRAY_NO_RETAIN
#ifdef	GSI_ARRAY_RETAIN
#undef	GSI_ARRAY_RETAIN
#endif
#define	GSI_ARRAY_RETAIN(A, X)	
#else
#ifndef	GSI_ARRAY_RETAIN
#define	GSI_ARRAY_RETAIN(A, X)	[(X).obj retain]
#endif
#endif

#ifdef	GSI_ARRAY_NO_RELEASE
#ifdef	GSI_ARRAY_RELEASE
#undef	GSI_ARRAY_RELEASE
#endif
#define	GSI_ARRAY_RELEASE(A, X)	
#else
#ifndef	GSI_ARRAY_RELEASE
#define	GSI_ARRAY_RELEASE(A, X)	[(X).obj release]
#endif
#endif

/*
 *	If there is no bitmask defined to supply the types that
 *	may be stored in the array, default to permitting all types.
 */
#ifndef	GSI_ARRAY_TYPES
#define	GSI_ARRAY_TYPES	GSUNION_ALL
#endif

/*
 *	Set up the name of the union to store array elements.
 */
#ifdef	GSUNION
#undef	GSUNION
#endif
#define	GSUNION	GSIArrayItem

/*
 *	Set up the types that will be storable in the union.
 *	See 'GSUnion.h' for details.
 */
#ifdef	GSUNION_TYPES
#undef	GSUNION_TYPES
#endif
#define	GSUNION_TYPES	GSI_ARRAY_TYPES
#ifdef	GSUNION_EXTRA
#undef	GSUNION_EXTRA
#endif

/*
 * Override extra type used in array value
 */
#ifdef	GSI_ARRAY_TYPE
#define	GSUNION_EXTRA	GSI_ARRAY_TYPE
#endif

/*
 *	Generate the union typedef
 */
#include <GNUstepBase/GSUnion.h>

struct	_GSIArray {
  GSIArrayItem	*ptr;
  unsigned	count;
  unsigned	cap;
  unsigned	old;
  NSZone	*zone;
#ifdef	GSI_ARRAY_EXTRA
  GSI_ARRAY_EXTRA	extra;
#endif
};
typedef	struct	_GSIArray	GSIArray_t;
typedef	struct	_GSIArray	*GSIArray;

static INLINE unsigned
GSIArrayCapacity(GSIArray array)
{
  return array->cap;
}

static INLINE unsigned
GSIArrayCount(GSIArray array)
{
  return array->count;
}

static INLINE void
GSIArrayGrow(GSIArray array)
{
  unsigned int	next;
  unsigned int	size;
  GSIArrayItem	*tmp;

  next = array->cap + array->old;
  size = next*sizeof(GSIArrayItem);
  tmp = NSZoneRealloc(array->zone, array->ptr, size);

  if (tmp == 0)
    {
      [NSException raise: NSMallocException
		  format: @"failed to grow GSIArray"];
    }
  array->ptr = tmp;
  array->old = array->cap;
  array->cap = next;
}

static INLINE void
GSIArrayGrowTo(GSIArray array, unsigned next)
{
  unsigned int	size;
  GSIArrayItem	*tmp;

  if (next < array->count)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"attempt to shrink below count"];
    }
  size = next*sizeof(GSIArrayItem);
  tmp = NSZoneRealloc(array->zone, array->ptr, size);

  if (tmp == 0)
    {
      [NSException raise: NSMallocException
		  format: @"failed to grow GSIArray"];
    }
  array->ptr = tmp;
  array->old = array->cap;
  array->cap = next;
}

static INLINE void
GSIArrayInsertItem(GSIArray array, GSIArrayItem item, unsigned index)
{
  unsigned int	i;

  GSI_ARRAY_RETAIN(array, item);
  GSI_ARRAY_CHECK;
  if (array->count == array->cap)
    {
      GSIArrayGrow(array);
    }
  for (i = array->count++; i > index; i--)
    {
      array->ptr[i] = array->ptr[i-1];
    }
  array->ptr[i] = item;
  GSI_ARRAY_CHECK;
}

static INLINE void
GSIArrayInsertItemNoRetain(GSIArray array, GSIArrayItem item, unsigned index)
{
  unsigned int	i;

  GSI_ARRAY_CHECK;
  if (array->count == array->cap)
    {
      GSIArrayGrow(array);
    }
  for (i = array->count++; i > index; i--)
    {
      array->ptr[i] = array->ptr[i-1];
    }
  array->ptr[i] = item;
  GSI_ARRAY_CHECK;
}

static INLINE void
GSIArrayAddItem(GSIArray array, GSIArrayItem item)
{
  GSI_ARRAY_RETAIN(array, item);
  GSI_ARRAY_CHECK;
  if (array->count == array->cap)
    {
      GSIArrayGrow(array);
    }
  array->ptr[array->count++] = item;
  GSI_ARRAY_CHECK;
}

static INLINE void
GSIArrayAddItemNoRetain(GSIArray array, GSIArrayItem item)
{
  GSI_ARRAY_CHECK;
  if (array->count == array->cap)
    {
      GSIArrayGrow(array);
    }
  array->ptr[array->count++] = item;
  GSI_ARRAY_CHECK;
}

/*
 *	The comparator function takes two items as arguments, the first is the
 *	item to be added, the second is the item already in the array.
 *      The function should return NSOrderedAscending if the item to be
 *      added is 'less than' the item in the array, NSOrderedDescending
 *      if it is greater, and NSOrderedSame if it is equal.
 */
static INLINE unsigned
GSIArrayInsertionPosition(GSIArray array, GSIArrayItem item, 
	NSComparisonResult (*sorter)(GSIArrayItem, GSIArrayItem))
{
  unsigned int	upper = array->count;
  unsigned int	lower = 0;
  unsigned int	index;

  /*
   *	Binary search for an item equal to the one to be inserted.
   */
  for (index = upper/2; upper != lower; index = (upper+lower)/2)
    {
      NSComparisonResult comparison;

      comparison = (*sorter)(item, (array->ptr[index]));
      if (comparison == NSOrderedAscending)
        {
          upper = index;
        }
      else if (comparison == NSOrderedDescending)
        {
          lower = index + 1;
        }
      else
        {
          break;
        }
    }
  /*
   *	Now skip past any equal items so the insertion point is AFTER any
   *	items that are equal to the new one.
   */
  while (index < array->count
    && (*sorter)(item, (array->ptr[index])) != NSOrderedAscending)
    {
      index++;
    }
#ifdef	GSI_ARRAY_CHECKS
  NSCAssert(index <= array->count, NSInternalInconsistencyException);
#endif
  return index;
}

#ifdef	GSI_ARRAY_CHECKS
static INLINE void
GSIArrayCheckSort(GSIArray array, 
	NSComparisonResult (*sorter)(GSIArrayItem, GSIArrayItem))
{
  unsigned int	i;

  for (i = 1; i < array->count; i++)
    {
#ifdef	GSI_ARRAY_CHECKS
      NSCAssert(((*sorter)(array->ptr[i-1], array->ptr[i]) 
#endif
	         != NSOrderedDecending), NSInvalidArgumentException);
    }
}
#endif

static INLINE void
GSIArrayInsertSorted(GSIArray array, GSIArrayItem item, 
	NSComparisonResult (*sorter)(GSIArrayItem, GSIArrayItem))
{
  unsigned int	index;

  index = GSIArrayInsertionPosition(array, item, sorter);
  GSIArrayInsertItem(array, item, index);
#ifdef	GSI_ARRAY_CHECKS
  GSIArrayCheckSort(array, sorter);
#endif
}

static INLINE void
GSIArrayInsertSortedNoRetain(GSIArray array, GSIArrayItem item,
	NSComparisonResult (*sorter)(GSIArrayItem, GSIArrayItem))
{
  unsigned int	index;

  index = GSIArrayInsertionPosition(array, item, sorter);
  GSIArrayInsertItemNoRetain(array, item, index);
#ifdef	GSI_ARRAY_CHECKS
  GSIArrayCheckSort(array, sorter);
#endif
}

static INLINE void
GSIArrayRemoveItemAtIndex(GSIArray array, unsigned index)
{
  GSIArrayItem	tmp;
#ifdef	GSI_ARRAY_CHECKS
  NSCAssert(index < array->count, NSInvalidArgumentException);
#endif
  tmp = array->ptr[index];
  while (++index < array->count)
    array->ptr[index-1] = array->ptr[index];
  array->count--;
  GSI_ARRAY_RELEASE(array, tmp);
}

static INLINE void
GSIArrayRemoveLastItem(GSIArray array)
{
#ifdef	GSI_ARRAY_CHECKS
  NSCAssert(array->count, NSInvalidArgumentException);
#endif
  GSI_ARRAY_RELEASE(array, array->ptr[array->count-1]);
  array->count--;
}

static INLINE void
GSIArrayRemoveItemAtIndexNoRelease(GSIArray array, unsigned index)
{
  GSIArrayItem	tmp;
#ifdef	GSI_ARRAY_CHECKS
  NSCAssert(index < array->count, NSInvalidArgumentException);
#endif
  tmp = array->ptr[index];
  while (++index < array->count)
    array->ptr[index-1] = array->ptr[index];
  array->count--;
}

static INLINE void
GSIArraySetItemAtIndex(GSIArray array, GSIArrayItem item, unsigned index)
{
  GSIArrayItem	tmp;
#ifdef	GSI_ARRAY_CHECKS
  NSCAssert(index < array->count, NSInvalidArgumentException);
#endif
  tmp = array->ptr[index];
  GSI_ARRAY_RETAIN(array, item);
  array->ptr[index] = item;
  GSI_ARRAY_RELEASE(array, tmp);
}

/*
 * For direct access ... unsafe if you change the array in any way while
 * examining the contents of this buffer.
 */
static INLINE GSIArrayItem *
GSIArrayItems(GSIArray array)
{
  return array->ptr;
}

static INLINE GSIArrayItem
GSIArrayItemAtIndex(GSIArray array, unsigned index)
{
#ifdef	GSI_ARRAY_CHECKS
  NSCAssert(index < array->count, NSInvalidArgumentException);
#endif
  return array->ptr[index];
}

static INLINE GSIArrayItem
GSIArrayLastItem(GSIArray array)
{
#ifdef	GSI_ARRAY_CHECKS
  NSCAssert(array->count, NSInvalidArgumentException);
#endif
  return array->ptr[array->count-1];
}

static INLINE void
GSIArrayClear(GSIArray array)
{
  if (array->ptr)
    {
      NSZoneFree(array->zone, (void*)array->ptr);
      array->ptr = 0;
      array->cap = 0;
    }
}

static INLINE void
GSIArrayRemoveItemsFromIndex(GSIArray array, unsigned index)
{
  if (index < array->count)
    {
#ifndef	GSI_ARRAY_NO_RELEASE
      while (array->count-- > index)
	{
	  GSI_ARRAY_RELEASE(array, array->ptr[array->count]);
	}
#endif
      array->count = index;
    }
}

static INLINE void
GSIArrayRemoveAllItems(GSIArray array)
{
#ifndef	GSI_ARRAY_NO_RELEASE
  while (array->count--)
    {
      GSI_ARRAY_RELEASE(array, array->ptr[array->count]);
    }
#endif
  array->count = 0;
}

static INLINE void
GSIArrayEmpty(GSIArray array)
{
  GSIArrayRemoveAllItems(array);
  GSIArrayClear(array);
}

static INLINE GSIArray
GSIArrayInitWithZoneAndCapacity(GSIArray array, NSZone *zone, size_t capacity)
{
  unsigned int	size;

  array->zone = zone;
  array->count = 0;
  if (capacity < 2)
    capacity = 2;
  array->cap = capacity;
  array->old = capacity/2;
  size = capacity*sizeof(GSIArrayItem);
  array->ptr = (GSIArrayItem*)NSZoneMalloc(zone, size);
  return array;
}

static INLINE GSIArray
GSIArrayCopyWithZone(GSIArray array, NSZone *zone)
{
  unsigned int i;
  GSIArray new;
  new = NSZoneMalloc(zone, sizeof(GSIArray_t));
  GSIArrayInitWithZoneAndCapacity(new, zone, array->count);

  for (i = 0; i < array->count; i++)
    {
      GSI_ARRAY_RETAIN(array, array->ptr[i]);
      new->ptr[new->count++] = array->ptr[i];
    }
  return new;
}