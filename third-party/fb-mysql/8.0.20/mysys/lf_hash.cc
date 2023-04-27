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

/**
  @file mysys/lf_hash.cc
  extensible hash

  @todo
     try to get rid of dummy nodes ?
     for non-unique hash, count only _distinct_ values
     (but how to do it in lf_hash_delete ?)
*/
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <atomic>

#include "lf.h"
#include "m_ctype.h"
#include "my_atomic.h"
#include "my_bit.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/mysys_priv.h"
#include "template_utils.h"

LF_REQUIRE_PINS(3)

/* An element of the list */
struct LF_SLIST {
  std::atomic<LF_SLIST *>
      link;      /* a pointer to the next element in a listand a flag */
  uint32 hashnr; /* reversed hash number, for sorting                 */
  const uchar *key;
  size_t keylen;
  /*
    data is stored here, directly after the keylen.
    thus the pointer to data is (void*)(slist_element_ptr+1)
  */
};

const int LF_HASH_OVERHEAD = sizeof(LF_SLIST);

/*
  a structure to pass the context (pointers two the three successive elements
  in a list) from my_lfind to linsert/ldelete
*/
typedef struct {
  std::atomic<LF_SLIST *> *prev;
  LF_SLIST *curr, *next;
} CURSOR;

/*
  the last bit in LF_SLIST::link is a "deleted" flag.
  the helper functions below convert it to a pure pointer or a pure flag
*/
template <class T>
static inline T *PTR(T *ptr) {
  intptr_t i = reinterpret_cast<intptr_t>(ptr);
  i &= (intptr_t)~1;
  return reinterpret_cast<T *>(i);
}

template <class T>
static inline bool DELETED(T *ptr) {
  const intptr_t i = reinterpret_cast<intptr_t>(ptr);
  return i & 1;
}

template <class T>
static inline T *SET_DELETED(T *ptr) {
  intptr_t i = reinterpret_cast<intptr_t>(ptr);
  i |= 1;
  return reinterpret_cast<T *>(i);
}

/*
  DESCRIPTION
    Search for hashnr/key/keylen in the list starting from 'head' and
    position the cursor. The list is ORDER BY hashnr, key

  RETURN
    0 - not found
    1 - found

  NOTE
    cursor is positioned in either case
    pins[0..2] are used, they are NOT removed on return
*/
static int my_lfind(std::atomic<LF_SLIST *> *head, CHARSET_INFO *cs,
                    uint32 hashnr, const uchar *key, size_t keylen,
                    CURSOR *cursor, LF_PINS *pins) {
  uint32 cur_hashnr;
  const uchar *cur_key;
  size_t cur_keylen;
  LF_SLIST *link;

retry:
  cursor->prev = head;
  do /* PTR() isn't necessary below, head is a dummy node */
  {
    cursor->curr = (LF_SLIST *)(*cursor->prev);
    lf_pin(pins, 1, cursor->curr);
  } while (*cursor->prev != cursor->curr && LF_BACKOFF);
  for (;;) {
    if (unlikely(!cursor->curr)) {
      return 0; /* end of the list */
    }
    do {
      /* QQ: XXX or goto retry ? */
      link = cursor->curr->link.load();
      cursor->next = PTR(link);
      lf_pin(pins, 0, cursor->next);
    } while (link != cursor->curr->link && LF_BACKOFF);
    cur_hashnr = cursor->curr->hashnr;
    cur_key = cursor->curr->key;
    cur_keylen = cursor->curr->keylen;
    if (*cursor->prev != cursor->curr) {
      (void)LF_BACKOFF;
      goto retry;
    }
    if (!DELETED(link)) {
      if (cur_hashnr >= hashnr) {
        int r = 1;
        if (cur_hashnr > hashnr ||
            (r = my_strnncoll(cs, cur_key, cur_keylen, key, keylen)) >= 0) {
          return !r;
        }
      }
      cursor->prev = &(cursor->curr->link);
      lf_pin(pins, 2, cursor->curr);
    } else {
      /*
        we found a deleted node - be nice, help the other thread
        and remove this deleted node
      */
      if (atomic_compare_exchange_strong(cursor->prev, &cursor->curr,
                                         cursor->next)) {
        lf_pinbox_free(pins, cursor->curr);
      } else {
        (void)LF_BACKOFF;
        goto retry;
      }
    }
    cursor->curr = cursor->next;
    lf_pin(pins, 1, cursor->curr);
  }
}

/**
  Search for list element satisfying condition specified by match
  function and position cursor on it.

  @param head          Head of the list to search in.
  @param first_hashnr  Hash value to start search from.
  @param last_hashnr   Hash value to stop search after.
  @param match         Match function.
  @param cursor        Cursor to be position.
  @param pins          LF_PINS for the calling thread to be used during
                       search for pinning result.

  @retval 0 - not found
  @retval 1 - found
*/

static int my_lfind_match(std::atomic<LF_SLIST *> *head, uint32 first_hashnr,
                          uint32 last_hashnr, lf_hash_match_func *match,
                          CURSOR *cursor, LF_PINS *pins) {
  uint32 cur_hashnr;
  LF_SLIST *link;

retry:
  cursor->prev = head;
  do /* PTR() isn't necessary below, head is a dummy node */
  {
    cursor->curr = (LF_SLIST *)(*cursor->prev);
    lf_pin(pins, 1, cursor->curr);
  } while (*cursor->prev != cursor->curr && LF_BACKOFF);
  for (;;) {
    if (unlikely(!cursor->curr)) {
      return 0; /* end of the list */
    }
    do {
      /* QQ: XXX or goto retry ? */
      link = cursor->curr->link.load();
      cursor->next = PTR(link);
      lf_pin(pins, 0, cursor->next);
    } while (link != cursor->curr->link && LF_BACKOFF);
    cur_hashnr = cursor->curr->hashnr;
    if (*cursor->prev != cursor->curr) {
      (void)LF_BACKOFF;
      goto retry;
    }
    if (!DELETED(link)) {
      if (cur_hashnr >= first_hashnr) {
        if (cur_hashnr > last_hashnr) {
          return 0;
        }

        if (cur_hashnr & 1) {
          /* Normal node. Check if element matches condition. */
          if ((*match)((uchar *)(cursor->curr + 1))) {
            return 1;
          }
        } else {
          /*
            Dummy node. Nothing to check here.

            Still thanks to the fact that dummy nodes are never deleted we
            can save it as a safe place to restart iteration if ever needed.
          */
          head = &cursor->curr->link;
        }
      }

      cursor->prev = &(cursor->curr->link);
      lf_pin(pins, 2, cursor->curr);
    } else {
      /*
        we found a deleted node - be nice, help the other thread
        and remove this deleted node
      */
      if (atomic_compare_exchange_strong(cursor->prev, &cursor->curr,
                                         cursor->next)) {
        lf_pinbox_free(pins, cursor->curr);
      } else {
        (void)LF_BACKOFF;
        goto retry;
      }
    }
    cursor->curr = cursor->next;
    lf_pin(pins, 1, cursor->curr);
  }
}

/*
  DESCRIPTION
    insert a 'node' in the list that starts from 'head' in the correct
    position (as found by my_lfind)

  RETURN
    0     - inserted
    not 0 - a pointer to a duplicate (not pinned and thus unusable)

  NOTE
    it uses pins[0..2], on return all pins are removed.
    if there're nodes with the same key value, a new node is added before them.
*/
static LF_SLIST *linsert(std::atomic<LF_SLIST *> *head, CHARSET_INFO *cs,
                         LF_SLIST *node, LF_PINS *pins, uint flags) {
  CURSOR cursor;
  int res;

  for (;;) {
    if (my_lfind(head, cs, node->hashnr, node->key, node->keylen, &cursor,
                 pins) &&
        (flags & LF_HASH_UNIQUE)) {
      res = 0; /* duplicate found */
      break;
    } else {
      node->link = cursor.curr;
      DBUG_ASSERT(node->link != node);         /* no circular references */
      DBUG_ASSERT(cursor.prev != &node->link); /* no circular references */
      if (atomic_compare_exchange_strong(cursor.prev, &cursor.curr, node)) {
        res = 1; /* inserted ok */
        break;
      }
    }
  }
  lf_unpin(pins, 0);
  lf_unpin(pins, 1);
  lf_unpin(pins, 2);
  /*
    Note that cursor.curr is not pinned here and the pointer is unreliable,
    the object may dissapear anytime. But if it points to a dummy node, the
    pointer is safe, because dummy nodes are never freed - initialize_bucket()
    uses this fact.
  */
  return res ? nullptr : cursor.curr;
}

/*
  DESCRIPTION
    deletes a node as identified by hashnr/keey/keylen from the list
    that starts from 'head'

  RETURN
    0 - ok
    1 - not found

  NOTE
    it uses pins[0..2], on return all pins are removed.
*/
static int ldelete(std::atomic<LF_SLIST *> *head, CHARSET_INFO *cs,
                   uint32 hashnr, const uchar *key, uint keylen,
                   LF_PINS *pins) {
  CURSOR cursor;
  int res;

  for (;;) {
    if (!my_lfind(head, cs, hashnr, key, keylen, &cursor, pins)) {
      res = 1; /* not found */
      break;
    } else {
      /* mark the node deleted */
      if (atomic_compare_exchange_strong(&cursor.curr->link, &cursor.next,
                                         SET_DELETED(cursor.next))) {
        /* and remove it from the list */
        if (atomic_compare_exchange_strong(cursor.prev, &cursor.curr,
                                           cursor.next)) {
          lf_pinbox_free(pins, cursor.curr);
        } else {
          /*
            somebody already "helped" us and removed the node ?
            Let's check if we need to help that someone too!
            (to ensure the number of "set DELETED flag" actions
            is equal to the number of "remove from the list" actions)
          */
          my_lfind(head, cs, hashnr, key, keylen, &cursor, pins);
        }
        res = 0;
        break;
      }
    }
  }
  lf_unpin(pins, 0);
  lf_unpin(pins, 1);
  lf_unpin(pins, 2);
  return res;
}

/*
  DESCRIPTION
    searches for a node as identified by hashnr/keey/keylen in the list
    that starts from 'head'

  RETURN
    0 - not found
    node - found

  NOTE
    it uses pins[0..2], on return the pin[2] keeps the node found
    all other pins are removed.
*/
static LF_SLIST *my_lsearch(std::atomic<LF_SLIST *> *head, CHARSET_INFO *cs,
                            uint32 hashnr, const uchar *key, uint keylen,
                            LF_PINS *pins) {
  CURSOR cursor;
  int res = my_lfind(head, cs, hashnr, key, keylen, &cursor, pins);
  if (res) {
    lf_pin(pins, 2, cursor.curr);
  }
  lf_unpin(pins, 0);
  lf_unpin(pins, 1);
  return res ? cursor.curr : nullptr;
}

static inline const uchar *hash_key(const LF_HASH *hash, const uchar *record,
                                    size_t *length) {
  if (hash->get_key) {
    return (*hash->get_key)(record, length);
  }
  *length = hash->key_length;
  return record + hash->key_offset;
}

/*
  Compute the hash key value from the raw key.

  @note, that the hash value is limited to 2^31, because we need one
  bit to distinguish between normal and dummy nodes.
*/
static inline uint calc_hash(LF_HASH *hash, const uchar *key, size_t keylen) {
  return (hash->hash_function(hash, key, keylen)) & INT_MAX32;
}

#define MAX_LOAD 1.0 /* average number of elements in a bucket */

static int initialize_bucket(LF_HASH *, std::atomic<LF_SLIST *> *, uint,
                             LF_PINS *);

/**
  Adaptor function which allows to use hash function from character
  set with LF_HASH.
*/
static uint cset_hash_sort_adapter(const LF_HASH *hash, const uchar *key,
                                   size_t length) {
  uint64 nr1 = 1, nr2 = 4;
  hash->charset->coll->hash_sort(hash->charset, key, length, &nr1, &nr2);
  return (uint)nr1;
}

/*
  Initializes lf_hash, the arguments are compatible with hash_init

  @note element_size sets both the size of allocated memory block for
  lf_alloc and a size of memcpy'ed block size in lf_hash_insert. Typically
  they are the same, indeed. But LF_HASH::element_size can be decreased
  after lf_hash_init, and then lf_alloc will allocate larger block that
  lf_hash_insert will copy over. It is desireable if part of the element
  is expensive to initialize - for example if there is a mutex or
  DYNAMIC_ARRAY. In this case they should be initialize in the
  LF_ALLOCATOR::constructor, and lf_hash_insert should not overwrite them.
  See wt_init() for example.
  As an alternative to using the above trick with decreasing
  LF_HASH::element_size one can provide an "initialize" hook that will finish
  initialization of object provided by LF_ALLOCATOR and set element key from
  object passed as parameter to lf_hash_insert instead of doing simple memcpy.
*/
void lf_hash_init2(LF_HASH *hash, uint element_size, uint flags,
                   uint key_offset, uint key_length,
                   hash_get_key_function get_key, CHARSET_INFO *charset,
                   lf_hash_func *hash_function, lf_allocator_func *ctor,
                   lf_allocator_func *dtor, lf_hash_init_func *init) {
  lf_alloc_init2(&hash->alloc, sizeof(LF_SLIST) + element_size,
                 offsetof(LF_SLIST, key), ctor, dtor);
  lf_dynarray_init(&hash->array, sizeof(LF_SLIST *));
  hash->size = 1;
  hash->count = 0;
  hash->element_size = element_size;
  hash->flags = flags;
  hash->charset = charset ? charset : &my_charset_bin;
  hash->key_offset = key_offset;
  hash->key_length = key_length;
  hash->get_key = get_key;
  hash->hash_function = hash_function ? hash_function : cset_hash_sort_adapter;
  hash->initialize = init;
  DBUG_ASSERT(get_key ? !key_offset && !key_length : key_length);
}

void lf_hash_destroy(LF_HASH *hash) {
  LF_SLIST *el, **head = (LF_SLIST **)lf_dynarray_value(&hash->array, 0);

  if (unlikely(!head)) {
    return;
  }
  el = *head;

  while (el) {
    LF_SLIST *next = el->link;
    if (el->hashnr & 1) {
      lf_alloc_direct_free(&hash->alloc, el); /* normal node */
    } else {
      my_free(el); /* dummy node */
    }
    el = (LF_SLIST *)next;
  }
  lf_alloc_destroy(&hash->alloc);
  lf_dynarray_destroy(&hash->array);
}

/*
  DESCRIPTION
    inserts a new element to a hash. it will have a _copy_ of
    data, not a pointer to it.

  RETURN
    0 - inserted
    1 - didn't (unique key conflict)
   -1 - out of memory

  NOTE
    see linsert() for pin usage notes
*/
int lf_hash_insert(LF_HASH *hash, LF_PINS *pins, const void *data) {
  int csize, bucket, hashnr;
  LF_SLIST *node;
  std::atomic<LF_SLIST *> *el;

  node = (LF_SLIST *)lf_alloc_new(pins);
  if (unlikely(!node)) {
    return -1;
  }
  uchar *extra_data =
      (uchar *)(node + 1);  // Stored immediately after the node.
  if (hash->initialize) {
    (*hash->initialize)(extra_data, (const uchar *)data);
  } else {
    memcpy(extra_data, data, hash->element_size);
  }
  node->key = hash_key(hash, (uchar *)(node + 1), &node->keylen);
  hashnr = calc_hash(hash, node->key, node->keylen);
  bucket = hashnr % hash->size;
  el = static_cast<std::atomic<LF_SLIST *> *>(
      lf_dynarray_lvalue(&hash->array, bucket));
  if (unlikely(!el)) {
    return -1;
  }
  if (el->load() == nullptr &&
      unlikely(initialize_bucket(hash, el, bucket, pins))) {
    return -1;
  }
  node->hashnr = my_reverse_bits(hashnr) | 1; /* normal node */
  if (linsert(el, hash->charset, node, pins, hash->flags)) {
    lf_pinbox_free(pins, node);
    return 1;
  }
  csize = hash->size;
  if ((hash->count.fetch_add(1) + 1.0) / csize > MAX_LOAD) {
    atomic_compare_exchange_strong(&hash->size, &csize, csize * 2);
  }
  return 0;
}

/*
  DESCRIPTION
    deletes an element with the given key from the hash (if a hash is
    not unique and there're many elements with this key - the "first"
    matching element is deleted)
  RETURN
    0 - deleted
    1 - didn't (not found)
   -1 - out of memory
  NOTE
    see ldelete() for pin usage notes
*/
int lf_hash_delete(LF_HASH *hash, LF_PINS *pins, const void *key, uint keylen) {
  std::atomic<LF_SLIST *> *el;
  uint bucket,
      hashnr = calc_hash(hash, pointer_cast<const uchar *>(key), keylen);

  bucket = hashnr % hash->size;
  el = static_cast<std::atomic<LF_SLIST *> *>(
      lf_dynarray_lvalue(&hash->array, bucket));
  if (unlikely(!el)) {
    return -1;
  }
  /*
    note that we still need to initialize_bucket here,
    we cannot return "node not found", because an old bucket of that
    node may've been split and the node was assigned to a new bucket
    that was never accessed before and thus is not initialized.
  */
  if (el->load() == nullptr &&
      unlikely(initialize_bucket(hash, el, bucket, pins))) {
    return -1;
  }
  if (ldelete(el, hash->charset, my_reverse_bits(hashnr) | 1,
              pointer_cast<const uchar *>(key), keylen, pins)) {
    return 1;
  }
  --hash->count;
  return 0;
}

/**
  Find hash element corresponding to the key.

  @param hash    The hash to search element in.
  @param pins    Pins for the calling thread which were earlier
                 obtained from this hash using lf_hash_get_pins().
  @param key     Key
  @param keylen  Key length

  @retval A pointer to an element with the given key (if a hash is not unique
          and there're many elements with this key - the "first" matching
          element).
  @retval NULL         - if nothing is found
  @retval MY_LF_ERRPTR - if OOM

  @note Uses pins[0..2]. On return pins[0..1] are removed and pins[2]
        is used to pin object found. It is also not removed in case when
        object is not found/error occurs but pin value is undefined in
        this case.
        So calling lf_hash_unpin() is mandatory after call to this function
        in case of both success and failure.
        @sa my_lsearch().
*/

void *lf_hash_search(LF_HASH *hash, LF_PINS *pins, const void *key,
                     uint keylen) {
  std::atomic<LF_SLIST *> *el;
  LF_SLIST *found;
  uint bucket,
      hashnr = calc_hash(hash, pointer_cast<const uchar *>(key), keylen);

  bucket = hashnr % hash->size;
  el = static_cast<std::atomic<LF_SLIST *> *>(
      lf_dynarray_lvalue(&hash->array, bucket));
  if (unlikely(!el)) {
    return MY_LF_ERRPTR;
  }
  if (el->load() == nullptr &&
      unlikely(initialize_bucket(hash, el, bucket, pins))) {
    return MY_LF_ERRPTR;
  }
  found = my_lsearch(el, hash->charset, my_reverse_bits(hashnr) | 1,
                     pointer_cast<const uchar *>(key), keylen, pins);
  return found ? found + 1 : nullptr;
}

/**
  Find random hash element which satisfies condition specified by
  match function.

  @param hash      Hash to search element in.
  @param pins      Pins for calling thread to be used during search
                   and for pinning its result.
  @param match     Pointer to match function. This function takes
                   pointer to object stored in hash as parameter
                   and returns 0 if object doesn't satisfy its
                   condition (and non-0 value if it does).
  @param rand_val  Random value to be used for selecting hash
                   bucket from which search in sort-ordered
                   list needs to be started.

  @retval A pointer to a random element matching condition.
  @retval NULL         - if nothing is found
  @retval MY_LF_ERRPTR - OOM.

  @note This function follows the same pinning protocol as lf_hash_search(),
        i.e. uses pins[0..2]. On return pins[0..1] are removed and pins[2]
        is used to pin object found. It is also not removed in case when
        object is not found/error occurs but its value is undefined in
        this case.
        So calling lf_hash_unpin() is mandatory after call to this function
        in case of both success and failure.
*/

void *lf_hash_random_match(LF_HASH *hash, LF_PINS *pins,
                           lf_hash_match_func *match, uint rand_val) {
  /* Convert random value to valid hash value. */
  uint hashnr = (rand_val & INT_MAX32);
  uint bucket;
  uint32 rev_hashnr;
  std::atomic<LF_SLIST *> *el;
  CURSOR cursor;
  int res;

  bucket = hashnr % hash->size;
  rev_hashnr = my_reverse_bits(hashnr);

  el = static_cast<std::atomic<LF_SLIST *> *>(
      lf_dynarray_lvalue(&hash->array, bucket));
  if (unlikely(!el)) {
    return MY_LF_ERRPTR;
  }
  /*
    Bucket might be totally empty if it has not been accessed since last
    time LF_HASH::size has been increased. In this case we initialize it
    by inserting dummy node for this bucket to the correct position in
    split-ordered list. This should help future lf_hash_* calls trying to
    access the same bucket.
  */
  if (el->load() == nullptr &&
      unlikely(initialize_bucket(hash, el, bucket, pins))) {
    return MY_LF_ERRPTR;
  }

  /*
    To avoid bias towards the first matching element in the bucket, we start
    looking for elements with inversed hash value greater or equal than
    inversed value of our random hash.
  */
  res = my_lfind_match(el, rev_hashnr | 1, UINT_MAX32, match, &cursor, pins);

  if (!res && hashnr != 0) {
    /*
      We have not found matching element - probably we were too close to
      the tail of our split-ordered list. To avoid bias against elements
      at the head of the list we restart our search from its head. Unless
      we were already searching from it.

      To avoid going through elements at which we have already looked
      twice we stop once we reach element from which we have begun our
      first search.
    */
    el = static_cast<std::atomic<LF_SLIST *> *>(
        lf_dynarray_lvalue(&hash->array, 0));
    if (unlikely(!el)) {
      return MY_LF_ERRPTR;
    }
    res = my_lfind_match(el, 1, rev_hashnr, match, &cursor, pins);
  }

  if (res) {
    lf_pin(pins, 2, cursor.curr);
  }
  lf_unpin(pins, 0);
  lf_unpin(pins, 1);

  return res ? cursor.curr + 1 : nullptr;
}

static const uchar *dummy_key = pointer_cast<const uchar *>("");

/*
  RETURN
    0 - ok
   -1 - out of memory
*/
static int initialize_bucket(LF_HASH *hash, std::atomic<LF_SLIST *> *node,
                             uint bucket, LF_PINS *pins) {
  uint parent = my_clear_highest_bit(bucket);
  LF_SLIST *dummy =
      (LF_SLIST *)my_malloc(key_memory_lf_slist, sizeof(LF_SLIST), MYF(MY_WME));
  LF_SLIST *tmp = nullptr, *cur;
  std::atomic<LF_SLIST *> *el = static_cast<std::atomic<LF_SLIST *> *>(
      lf_dynarray_lvalue(&hash->array, parent));
  if (unlikely(!el || !dummy)) {
    return -1;
  }
  if (el->load() == nullptr && bucket &&
      unlikely(initialize_bucket(hash, el, parent, pins))) {
    return -1;
  }
  dummy->hashnr = my_reverse_bits(bucket) | 0; /* dummy node */
  dummy->key = dummy_key;
  dummy->keylen = 0;
  if ((cur = linsert(el, hash->charset, dummy, pins, LF_HASH_UNIQUE))) {
    my_free(dummy);
    dummy = cur;
  }
  atomic_compare_exchange_strong(node, &tmp, dummy);
  /*
    note that if the CAS above failed (after linsert() succeeded),
    it would mean that some other thread has executed linsert() for
    the same dummy node, its linsert() failed, it picked up our
    dummy node (in "dummy= cur") and executed the same CAS as above.
    Which means that even if CAS above failed we don't need to retry,
    and we should not free(dummy) - there's no memory leak here
  */
  return 0;
}
