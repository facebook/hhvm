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

#include <runtime/base/memory/linear_allocator.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

LinearAllocator::LinearAllocator()
  : m_blob(NULL), m_size(0), m_pos(0), m_count(0), m_index(0), m_item(NULL) {
}

LinearAllocator::~LinearAllocator() {
  if (m_blob) {
    free(m_blob);
  }
}

void LinearAllocator::reset() {
  if (m_blob) {
    free(m_blob);
    m_blob = NULL;
  }
  m_size = 0;
  m_pos = 0;
  m_items.clear();
  m_count = 0;
  m_index = 0;
}

void LinearAllocator::beginBackup(int size, int count) {
  if (m_blob) {
    free(m_blob);
    m_blob = NULL;
  }
  if (size > 0) {
    ASSERT(count > 0);
    m_blob = (char*)malloc(size);
  } else {
    ASSERT(size == 0);
    ASSERT(count >= 0); // count can be non-zero for NULL terminiations
  }
  m_size = size;
  m_pos = 0;

  m_items.clear(); // so it will be filled with 0s when resize
  m_items.resize(count);
  m_count = count;
  m_index = 0;
  m_item = NULL;
}

void LinearAllocator::backup(void *p) {
  ASSERT(m_index < m_count);
  ItemInfo &item = m_items[m_index++];
  item.p = p;
  item.end_pos = m_pos;
  m_item = &item;
}

void LinearAllocator::backup(int size) {
  backup((const char *)&size, sizeof(size));
}

void LinearAllocator::backup(const char *data, int size) {
  ASSERT(data);
  ASSERT(size);
  ASSERT(m_item);

  ASSERT(m_pos + size <= m_size);
  memcpy(m_blob + m_pos, data, size);
  m_pos += size;
  m_item->end_pos = m_pos;
}

void LinearAllocator::endBackup() {
  ASSERT(m_pos == m_size);
  ASSERT(m_index == m_count);
  m_item = NULL;
}

void LinearAllocator::beginRestore() {
  ASSERT(m_pos == 0 || m_pos == m_size);
  ASSERT(m_index == 0 || m_index == m_count);

  m_pos = 0;
  m_index = 0;
}

void *LinearAllocator::restore(const char *&data) {
  ASSERT(m_pos <= m_size);
  ASSERT(m_index < m_count);

  ItemInfo &item = m_items[m_index++];
  data = m_blob + m_pos;
  m_pos = item.end_pos;
  return item.p;
}

void LinearAllocator::endRestore() {
  ASSERT(m_pos == 0 || m_pos == m_size);
  ASSERT(m_index == 0 || m_index == m_count);
}

void LinearAllocator::checkMemory(bool detailed) {
  printf("LinearAllocator: size = %d, pos = %d, count = %d, index = %d\n",
         m_size, m_pos, m_count, m_index);
}

///////////////////////////////////////////////////////////////////////////////
}
