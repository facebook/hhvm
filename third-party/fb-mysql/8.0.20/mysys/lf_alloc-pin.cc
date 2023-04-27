/* QQ: TODO multi-pinbox */
/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <stddef.h>
#include <sys/types.h>
#include <atomic>

static_assert(sizeof(std::atomic<void *>) == sizeof(void *),
              "We happily cast to and from std::atomic<void *>, so they need "
              "to be at least"
              "nominally compatible.");

/**
  @file mysys/lf_alloc-pin.cc
  wait-free concurrent allocator based on pinning addresses.

  It works as follows: every thread (strictly speaking - every CPU, but
  it's too difficult to do) has a small array of pointers. They're called
  "pins".  Before using an object its address must be stored in this array
  (pinned).  When an object is no longer necessary its address must be
  removed from this array (unpinned). When a thread wants to free() an
  object it scans all pins of all threads to see if somebody has this
  object pinned.  If yes - the object is not freed (but stored in a
  "purgatory").  To reduce the cost of a single free() pins are not scanned
  on every free() but only added to (thread-local) purgatory. On every
  LF_PURGATORY_SIZE free() purgatory is scanned and all unpinned objects
  are freed.

  Pins are used to solve ABA problem. To use pins one must obey
  a pinning protocol:

   1. Let's assume that PTR is a shared pointer to an object. Shared means
      that any thread may modify it anytime to point to a different object
      and free the old object. Later the freed object may be potentially
      allocated by another thread. If we're unlucky that other thread may
      set PTR to point to this object again. This is ABA problem.
   2. Create a local pointer LOCAL_PTR.
   3. Pin the PTR in a loop:
      do
      {
        LOCAL_PTR= PTR;
        pin(PTR, PIN_NUMBER);
      } while (LOCAL_PTR != PTR)
   4. It is guaranteed that after the loop has ended, LOCAL_PTR
      points to an object (or NULL, if PTR may be NULL), that
      will never be freed. It is not guaranteed though
      that LOCAL_PTR == PTR (as PTR can change any time)
   5. When done working with the object, remove the pin:
      unpin(PIN_NUMBER)
   6. When copying pins (as in the list traversing loop:
        pin(CUR, 1);
        while ()
        {
          do                            // standard
          {                             //  pinning
            NEXT=CUR->next;             //   loop
            pin(NEXT, 0);               //    see #3
          } while (NEXT != CUR->next);  //     above
          ...
          ...
          CUR=NEXT;
          pin(CUR, 1);                  // copy pin[0] to pin[1]
        }
      which keeps CUR address constantly pinned), note than pins may be
      copied only upwards (!!!), that is pin[N] to pin[M], M > N.
   7. Don't keep the object pinned longer than necessary - the number of
      pins you have is limited (and small), keeping an object pinned
      prevents its reuse and cause unnecessary mallocs.

  Explanations:

   3. The loop is important. The following can occur:
        thread1> LOCAL_PTR= PTR
        thread2> free(PTR); PTR=0;
        thread1> pin(PTR, PIN_NUMBER);
      now thread1 cannot access LOCAL_PTR, even if it's pinned,
      because it points to a freed memory. That is, it *must*
      verify that it has indeed pinned PTR, the shared pointer.

   6. When a thread wants to free some LOCAL_PTR, and it scans
      all lists of pins to see whether it's pinned, it does it
      upwards, from low pin numbers to high. Thus another thread
      must copy an address from one pin to another in the same
      direction - upwards, otherwise the scanning thread may
      miss it.

  Implementation details:

  Pins are given away from a "pinbox". Pinbox is stack-based allocator.
  It used dynarray for storing pins, new elements are allocated by dynarray
  as necessary, old are pushed in the stack for reuse. ABA is solved by
  versioning a pointer - because we use an array, a pointer to pins is 16 bit,
  upper 16 bits are used for a version.
*/
#include "lf.h"
#include "my_atomic.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "my_thread.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/mysys_priv.h" /* key_memory_lf_node */

#define LF_PINBOX_MAX_PINS 65536

static void lf_pinbox_real_free(LF_PINS *pins);

/*
  Initialize a pinbox. Normally called from lf_alloc_init.
  See the latter for details.
*/
void lf_pinbox_init(LF_PINBOX *pinbox, uint free_ptr_offset,
                    lf_pinbox_free_func *free_func, void *free_func_arg) {
  DBUG_ASSERT(free_ptr_offset % sizeof(void *) == 0);
  static_assert(sizeof(LF_PINS) == 64, "");
  lf_dynarray_init(&pinbox->pinarray, sizeof(LF_PINS));
  pinbox->pinstack_top_ver = 0;
  pinbox->pins_in_array = 0;
  pinbox->free_ptr_offset = free_ptr_offset;
  pinbox->free_func = free_func;
  pinbox->free_func_arg = free_func_arg;
}

void lf_pinbox_destroy(LF_PINBOX *pinbox) {
  lf_dynarray_destroy(&pinbox->pinarray);
}

/*
  Get pins from a pinbox.

  SYNOPSYS
    pinbox      -

  DESCRIPTION
    get a new LF_PINS structure from a stack of unused pins,
    or allocate a new one out of dynarray.
*/
LF_PINS *lf_pinbox_get_pins(LF_PINBOX *pinbox) {
  uint32 pins, next, top_ver;
  LF_PINS *el;
  /*
    We have an array of max. 64k elements.
    The highest index currently allocated is pinbox->pins_in_array.
    Freed elements are in a lifo stack, pinstack_top_ver.
    pinstack_top_ver is 32 bits; 16 low bits are the index in the
    array, to the first element of the list. 16 high bits are a version
    (every time the 16 low bits are updated, the 16 high bits are
    incremented). Versioning prevents the ABA problem.
  */
  top_ver = pinbox->pinstack_top_ver;
  do {
    if (!(pins = top_ver % LF_PINBOX_MAX_PINS)) {
      /* the stack of free elements is empty */
      pins = pinbox->pins_in_array.fetch_add(1) + 1;
      if (unlikely(pins >= LF_PINBOX_MAX_PINS)) {
        return nullptr;
      }
      /*
        note that the first allocated element has index 1 (pins==1).
        index 0 is reserved to mean "NULL pointer"
      */
      el = (LF_PINS *)lf_dynarray_lvalue(&pinbox->pinarray, pins);
      if (unlikely(!el)) {
        return nullptr;
      }
      break;
    }
    el = (LF_PINS *)lf_dynarray_value(&pinbox->pinarray, pins);
    next = el->link;
  } while (!atomic_compare_exchange_strong(
      &pinbox->pinstack_top_ver, &top_ver,
      top_ver - pins + next + LF_PINBOX_MAX_PINS));
  /*
    set el->link to the index of el in the dynarray (el->link has two usages:
    - if element is allocated, it's its own index
    - if element is free, it's its next element in the free stack
  */
  el->link = pins;
  el->purgatory_count = 0;
  el->pinbox = pinbox;
  return el;
}

/*
  Put pins back to a pinbox.

  DESCRIPTION
    empty the purgatory (XXX deadlock warning below!),
    push LF_PINS structure to a stack
*/
void lf_pinbox_put_pins(LF_PINS *pins) {
  LF_PINBOX *pinbox = pins->pinbox;
  uint32 top_ver, nr;
  nr = pins->link;

#ifndef DBUG_OFF
  {
    /* This thread should not hold any pin. */
    int i;
    for (i = 0; i < LF_PINBOX_PINS; i++) {
      DBUG_ASSERT(pins->pin[i] == nullptr);
    }
  }
#endif /* DBUG_OFF */

  /*
    XXX this will deadlock if other threads will wait for
    the caller to do something after _lf_pinbox_put_pins(),
    and they would have pinned addresses that the caller wants to free.
    Thus: only free pins when all work is done and nobody can wait for you!!!
  */
  while (pins->purgatory_count) {
    lf_pinbox_real_free(pins);
    if (pins->purgatory_count) {
      my_thread_yield();
    }
  }
  top_ver = pinbox->pinstack_top_ver;
  do {
    pins->link = top_ver % LF_PINBOX_MAX_PINS;
  } while (!atomic_compare_exchange_strong(
      &pinbox->pinstack_top_ver, &top_ver,
      top_ver - pins->link + nr + LF_PINBOX_MAX_PINS));
}

/*
  Get the next pointer in the purgatory list.
  Note that next_node is not used to avoid the extra volatile.
*/
#define pnext_node(P, X) (*((void **)(((char *)(X)) + (P)->free_ptr_offset)))

static inline void add_to_purgatory(LF_PINS *pins, void *addr) {
  pnext_node(pins->pinbox, addr) = pins->purgatory;
  pins->purgatory = addr;
  pins->purgatory_count++;
}

/*
  Free an object allocated via pinbox allocator

  DESCRIPTION
    add an object to purgatory. if necessary, call lf_pinbox_real_free()
    to actually free something.
*/
void lf_pinbox_free(LF_PINS *pins, void *addr) {
  add_to_purgatory(pins, addr);
  if (pins->purgatory_count % LF_PURGATORY_SIZE == 0) {
    lf_pinbox_real_free(pins);
  }
}

struct st_match_and_save_arg {
  LF_PINS *pins;
  LF_PINBOX *pinbox;
  void *old_purgatory;
};

/*
  Callback for lf_dynarray_iterate:
  Scan all pins of all threads, for each active (non-null) pin,
  scan the current thread's purgatory. If present there, move it
  to a new purgatory. At the end, the old purgatory will contain
  pointers not pinned by any thread.
*/
static int match_and_save(void *v_el, void *v_arg) {
  LF_PINS *el = static_cast<LF_PINS *>(v_el);
  st_match_and_save_arg *arg = static_cast<st_match_and_save_arg *>(v_arg);
  int i;
  LF_PINS *el_end = el + LF_DYNARRAY_LEVEL_LENGTH;
  for (; el < el_end; el++) {
    for (i = 0; i < LF_PINBOX_PINS; i++) {
      void *p = el->pin[i];
      if (p) {
        void *cur = arg->old_purgatory;
        void **list_prev = &arg->old_purgatory;
        while (cur) {
          void *next = pnext_node(arg->pinbox, cur);

          if (p == cur) {
            /* pinned - keeping */
            add_to_purgatory(arg->pins, cur);
            /* unlink from old purgatory */
            *list_prev = next;
          } else {
            list_prev = (void **)((char *)cur + arg->pinbox->free_ptr_offset);
          }
          cur = next;
        }
        if (!arg->old_purgatory) {
          return 1;
        }
      }
    }
  }
  return 0;
}

/*
  Scan the purgatory and free everything that can be freed
*/
static void lf_pinbox_real_free(LF_PINS *pins) {
  LF_PINBOX *pinbox = pins->pinbox;

  /* Store info about current purgatory. */
  struct st_match_and_save_arg arg = {pins, pinbox, pins->purgatory};
  /* Reset purgatory. */
  pins->purgatory = nullptr;
  pins->purgatory_count = 0;

  lf_dynarray_iterate(&pinbox->pinarray, match_and_save, &arg);

  if (arg.old_purgatory) {
    /* Some objects in the old purgatory were not pinned, free them. */
    void *last = arg.old_purgatory;
    while (pnext_node(pinbox, last)) {
      last = pnext_node(pinbox, last);
    }
    pinbox->free_func(arg.old_purgatory, last, pinbox->free_func_arg);
  }
}

static inline std::atomic<uchar *> &next_node(LF_PINBOX *P, uchar *X) {
  std::atomic<uchar *> *free_ptr =
      (std::atomic<uchar *> *)(X + P->free_ptr_offset);
  return *free_ptr;
}

#define anext_node(X) next_node(&allocator->pinbox, (X))

/* lock-free memory allocator for fixed-size objects */

LF_REQUIRE_PINS(1)

/*
  callback for lf_pinbox_real_free to free a list of unpinned objects -
  add it back to the allocator stack

  DESCRIPTION
    'first' and 'last' are the ends of the linked list of nodes:
    first->el->el->....->el->last. Use first==last to free only one element.
*/
static void alloc_free(void *v_first, void *v_last, void *v_allocator) {
  uchar *first = static_cast<uchar *>(v_first);
  uchar *last = static_cast<uchar *>(v_last);
  LF_ALLOCATOR *allocator = static_cast<LF_ALLOCATOR *>(v_allocator);
  uchar *node = allocator->top;
  do {
    anext_node(last) = node;
  } while (!atomic_compare_exchange_strong(&allocator->top, &node, first) &&
           LF_BACKOFF);
}

/**
  Initialize lock-free allocator.

  @param  allocator           Allocator structure to initialize.
  @param  size                A size of an object to allocate.
  @param  free_ptr_offset     An offset inside the object to a sizeof(void *)
                              memory that is guaranteed to be unused after
                              the object is put in the purgatory. Unused by
                              ANY thread, not only the purgatory owner.
                              This memory will be used to link
                              waiting-to-be-freed objects in a purgatory list.
  @param ctor                 Function to be called after object was
                              malloc()'ed.
  @param dtor                 Function to be called before object is free()'d.
*/

void lf_alloc_init2(LF_ALLOCATOR *allocator, uint size, uint free_ptr_offset,
                    lf_allocator_func *ctor, lf_allocator_func *dtor) {
  lf_pinbox_init(&allocator->pinbox, free_ptr_offset, alloc_free, allocator);
  allocator->top = nullptr;
  allocator->mallocs = 0;
  allocator->element_size = size;
  allocator->constructor = ctor;
  allocator->destructor = dtor;
  DBUG_ASSERT(size >= sizeof(void *) + free_ptr_offset);
}

/*
  destroy the allocator, free everything that's in it

  NOTE
    As every other init/destroy function here and elsewhere it
    is not thread safe. No, this function is no different, ensure
    that no thread needs the allocator before destroying it.
    We are not responsible for any damage that may be caused by
    accessing the allocator when it is being or has been destroyed.
    Oh yes, and don't put your cat in a microwave.
*/
void lf_alloc_destroy(LF_ALLOCATOR *allocator) {
  uchar *node = allocator->top;
  while (node) {
    uchar *tmp = anext_node(node);
    if (allocator->destructor) {
      allocator->destructor(node);
    }
    my_free(node);
    node = tmp;
  }
  lf_pinbox_destroy(&allocator->pinbox);
  allocator->top = nullptr;
}

/*
  Allocate and return an new object.

  DESCRIPTION
    Pop an unused object from the stack or malloc it is the stack is empty.
    pin[0] is used, it's removed on return.
*/
void *lf_alloc_new(LF_PINS *pins) {
  LF_ALLOCATOR *allocator = (LF_ALLOCATOR *)(pins->pinbox->free_func_arg);
  uchar *node;
  for (;;) {
    do {
      node = allocator->top;
      lf_pin(pins, 0, node);
    } while (node != allocator->top && LF_BACKOFF);
    if (!node) {
      node = static_cast<uchar *>(
          my_malloc(key_memory_lf_node, allocator->element_size, MYF(MY_WME)));
      if (allocator->constructor) {
        allocator->constructor(node);
      }
#ifdef MY_LF_EXTRA_DEBUG
      if (likely(node != 0)) {
        ++allocator->mallocs;
      }
#endif
      break;
    }
    if (atomic_compare_exchange_strong(&allocator->top, &node,
                                       anext_node(node).load())) {
      break;
    }
  }
  lf_unpin(pins, 0);
  return node;
}

/*
  count the number of objects in a pool.

  NOTE
    This is NOT thread-safe !!!
*/
uint lf_alloc_pool_count(LF_ALLOCATOR *allocator) {
  uint i;
  uchar *node;
  for (node = allocator->top, i = 0; node; node = anext_node(node), i++)
    /* no op */;
  return i;
}
