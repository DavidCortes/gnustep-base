/* Interface for NSAutoreleasePool for GNUStep
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Date: 1995
   
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA.
   */ 

#ifndef __NSAutoreleasePool_h_GNUSTEP_BASE_INCLUDE
#define __NSAutoreleasePool_h_GNUSTEP_BASE_INCLUDE

#include <Foundation/NSObject.h>

@class NSAutoreleasePool;
@class NSThread;


/* Each thread has its own copy of these variables.
   A pointer to this structure is an ivar of NSThread. */
typedef struct autorelease_thread_vars
{
  /* The current, default NSAutoreleasePool for the calling thread;
     the one that will hold objects that are arguments to
     [NSAutoreleasePool +addObject:]. */
  NSAutoreleasePool *current_pool;

  /* The total number of objects autoreleased since the thread was
     started, or since -resetTotalAutoreleasedObjects was called
     in this thread. (if compiled in) */
  unsigned total_objects_count;

  /* A cache of NSAutoreleasePool's already alloc'ed.  Caching old pools
     instead of deallocating and re-allocating them will save time. */
  id *pool_cache;
  int pool_cache_size;
  int pool_cache_count;
} thread_vars_struct;

/* Initialize an autorelease_thread_vars structure for a new thread.
   This function is called in NSThread each time an NSThread is created.
   TV should be of type `struct autorelease_thread_vars *' */
#define init_autorelease_thread_vars(TV)  memset (TV, 0, sizeof (typeof (*TV)))



/* Each pool holds its objects-to-be-released in a linked-list of 
   these structures. */
typedef struct autorelease_array_list
{
  struct autorelease_array_list *next;
  unsigned size;
  unsigned count;
  id objects[0];
} array_list_struct;


@interface NSAutoreleasePool : NSObject 
{
  /* For re-setting the current pool when we are dealloc'ed. */
  NSAutoreleasePool *_parent;
  /* This pointer to our child pool is  necessary for co-existing
     with exceptions. */
  NSAutoreleasePool *_child;
  /* A collection of the objects to be released. */
  struct autorelease_array_list *_released;
  struct autorelease_array_list *_released_head;
  /* The total number of objects autoreleased in this pool. */
  unsigned _released_count;
  /* The method to add an object to this pool */
  void 	(*_addImp)(id, SEL, id);
}

+ (void) addObject: (id)anObj;
- (void) addObject: (id)anObj;

#ifndef	NO_GNUSTEP
+ (void) enableRelease: (BOOL)enable;
+ (void) freeCache;	/* Free cache of unused pools in this thread.	*/
+ (void) setPoolCountThreshhold: (unsigned)c;
+ (unsigned) autoreleaseCountForObject: (id)anObject;
+ (void) _endThread: (NSThread*)thread; /* Don't call this directly. */
/*
 * The next two methods have no effect unless you define COUNT_ALL to be
 * 1 in NSAutoreleasepool.m - doing so incurs a thread lookup overhead
 * each time an object is autoreleased.
 */
+ (void) resetTotalAutoreleasedObjects;
+ (unsigned) totalAutoreleasedObjects;
#endif
@end

#endif /* __NSAutoreleasePool_h_GNUSTEP_BASE_INCLUDE */