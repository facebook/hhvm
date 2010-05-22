/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef __CHUNK_LIST_H__
#define __CHUNK_LIST_H__

#include <stdlib.h>
#include <string.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template<typename T, unsigned int B>
class ChunkList {

#define CHUNK_ITEM_COUNT ((B - sizeof(void *) * 2) / sizeof(T))

private:
  class ChunkBase {
   public:
    ChunkBase *prev, *next;
  };

  class Chunk : public ChunkBase {
   public:
    T chunk[CHUNK_ITEM_COUNT];
  };

public:
  ChunkList() : m_head(&m_sentinel), m_tail(m_head),
                m_size(0), m_pos(CHUNK_ITEM_COUNT - 1), m_capacity(0),
                m_end(this, m_head, 0) {
    m_head->prev = m_head->next = m_head;
  }

  ~ChunkList() {
    releaseAll();
  }

  unsigned int size() const { return m_size; }

  unsigned int capacity() const { return m_capacity; }

  // Clears the list, and releases all but the sentinel.
  void clear() {
    releaseAll();
    m_tail = m_head->next = m_head->prev = m_head;
    m_size = m_capacity = 0;
    m_pos = CHUNK_ITEM_COUNT - 1;
  }

  // Adjusts the reserved storage space.
  void reserve(unsigned int s) {
    if (s == 0) {
      clear();
      return;
    }
    if (s <= m_capacity) {
      while (m_capacity >= CHUNK_ITEM_COUNT + s) {
        releaseChunk();
      }
      if (s < m_size) {
        m_size = s;
        m_pos = (s + CHUNK_ITEM_COUNT - 1) % CHUNK_ITEM_COUNT;
        m_tail = m_head->prev;
      }
    } else {
      while (m_capacity < s) {
        allocChunk();
      }
    }
  }

  void copy(const ChunkList<T, B> &source) {
    unsigned int s = source.size();
    reserve(s);
    if (s == 0) return;
    ChunkBase *p = m_head->next;
    ChunkBase *ps = source.m_head->next;
    while (p != m_head) {
      memcpy(((Chunk *)p)->chunk, ((Chunk *)ps)->chunk,
             sizeof(T) * CHUNK_ITEM_COUNT);
      p = p->next;
      ps = ps->next;
    }
    m_size = s;
    m_pos = source.m_pos;
    m_tail = m_head->prev;
  }

  ChunkList<T, B> &operator=(const ChunkList<T, B> &source) {
    copy(source);
    return *this;
  }

  void push_back(T t) {
    m_size++;
    m_pos++;
    if (m_pos < CHUNK_ITEM_COUNT) {
      ((Chunk *)m_tail)->chunk[m_pos] = t;
    } else {
      if (m_size > m_capacity) allocChunk();
      m_pos = 0;
      m_tail = m_tail->next;
      ((Chunk *)m_tail)->chunk[m_pos] = t;
    }
  }

  T back() const {
    ASSERT(m_size > 0);
    return ((Chunk *)m_tail)->chunk[m_pos];
  }

  void pop_back() {
    if (m_size == 0) return;
    m_size--;
    if (m_pos) {
      m_pos--;
    } else {
      m_pos = CHUNK_ITEM_COUNT - 1;
      m_tail = m_tail->prev;
    }
  }

  class Iterator {
   public:
    Iterator(const ChunkList *l = NULL, ChunkBase *c = NULL,
             unsigned int p = 0)
      : m_list(l), m_chunk(c), m_pos(p) { }

    Iterator(const Iterator &other) {
      m_list = other.m_list;
      m_chunk = other.m_chunk;
      m_pos = other.m_pos;
    }

    bool operator==(const Iterator &it) {
      return m_list == it.m_list && m_chunk == it.m_chunk && m_pos == it.m_pos;
    }

    bool operator!=(const Iterator &it) {
      return !operator==(it);
    }

    Iterator &operator++() {
      if (!m_list || operator==(m_list->m_end)) return *this;
      if (m_chunk) {
        m_pos++;
        if (m_chunk == m_list->m_tail && m_pos > m_list->m_pos) {
          operator=(m_list->m_end);
        } else if (m_pos == CHUNK_ITEM_COUNT) {
          m_pos = 0;
          m_chunk = m_chunk->next;
        }
      }
      return *this;
    }

    Iterator operator++(int) {
      Iterator ret(*this);
      operator++();
      return ret;
    }

    T &operator*() const {
      ASSERT(m_chunk != m_list->m_head);
      return ((Chunk *)m_chunk)->chunk[m_pos];
    }

    T *operator->() const {
      ASSERT(m_chunk != m_list->m_head);
      return ((Chunk *)m_chunk)->chunk + m_pos;
    }

   private:
    const ChunkList *m_list;
    ChunkBase *m_chunk;
    unsigned int m_pos;
  };

  Iterator begin() {
    if (m_size == 0) return end();
    return Iterator(this, m_head->next, 0);
  }

  const Iterator &end() const {
    return m_end;
  }

  typedef Iterator iterator;

private:
  void allocChunk() {
    Chunk *p = (Chunk *)malloc(sizeof(Chunk));
    p->prev = m_head->prev;
    p->next = m_head;
    m_head->prev->next = (ChunkBase *)p;
    m_head->prev = (ChunkBase *)p;
    m_capacity += CHUNK_ITEM_COUNT;
  }

  void releaseChunk() {
    if (m_capacity == 0) return;
    Chunk *p = (Chunk *)m_head->prev;
    m_head->prev = p->prev;
    m_head->prev->next = m_head;
    free(p);
    m_capacity -= CHUNK_ITEM_COUNT;
  }

  void releaseAll() {
    ChunkBase *p = m_head->next;
    while (p != m_head) {
      Chunk *tmp = (Chunk *)p;
      p = p->next;
      free(tmp);
    }
  }

  ChunkBase m_sentinel;
  ChunkBase *m_head;
  ChunkBase *m_tail;      // logical tail
  unsigned int m_size;
  unsigned int m_pos;     // position of the last element in its chunk
  unsigned int m_capacity;
  const Iterator m_end;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __CHUNK_LIST_H__ */
