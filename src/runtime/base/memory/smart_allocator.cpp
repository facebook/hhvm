/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/memory/memory_manager.h>
#include <runtime/base/resource_data.h>
#include <runtime/base/server/server_stats.h>
#include <runtime/base/runtime_option.h>
#include <util/logger.h>

using namespace std;
using namespace boost;

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// initializer

std::set<AllocatorThreadLocalInit>& GetAllocatorInitList() {
  static std::set<AllocatorThreadLocalInit> allocatorInitList;
  return allocatorInitList;
}

void InitAllocatorThreadLocal() {
  for (std::set<AllocatorThreadLocalInit>::iterator it =
      GetAllocatorInitList().begin();
      it != GetAllocatorInitList().end(); it++) {
    (*it)();
  }
}

///////////////////////////////////////////////////////////////////////////////
// helpers

static int calculate_item_count(int itemSize) {
  int itemCount = SLAB_SIZE / itemSize;
  if (itemCount == 0) {
    itemCount = 1;
  } else if (itemCount > MAX_OBJECT_COUNT_PER_SLAB) {
    itemCount = MAX_OBJECT_COUNT_PER_SLAB;
  }
  return itemCount;
}

static int findIndex(const vector<char *> &blocks,
                     const BlockIndexMap &blockIndex, int64 p, int colMax) {
  // First try
  int64 hit = p / colMax;
  BlockIndexMap::const_iterator it = blockIndex.find(hit);
  if (it != blockIndex.end()) {
    int idx = it->second;
    if ((int64)blocks[idx] <= p) {
      return idx;
    }
  }
  // Second try, and it must be correct
  hit--;
  ASSERT(blockIndex.find(hit) != blockIndex.end() &&
         (int64)blocks[blockIndex.find(hit)->second] <= p);
  return blockIndex.find(hit)->second;
}

#ifdef SMART_ALLOCATOR_STACKTRACE
Mutex SmartAllocatorImpl::s_st_mutex;
std::map<void*, StackTrace> SmartAllocatorImpl::s_st_allocs;
std::map<void*, StackTrace> SmartAllocatorImpl::s_st_deallocs;
#endif

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

SmartAllocatorImpl::SmartAllocatorImpl(int nameEnum, int itemCount,
                                       int itemSize, int flag)
  : m_itemCount(itemCount), m_itemSize(itemSize), m_flag(flag),
    m_row(0), m_col(0),
    m_rowChecked(0), m_colChecked(0), m_linearSize(0), m_linearCount(0),
    m_allocatedBlocks(0), m_multiplier(1), m_maxMultiplier(1),
    m_targetMultiplier(1),
    m_iter(this), m_linearized(false), m_stats(NULL) {

  // automatically pick a good per slab item count
  if (m_itemCount <= 0) {
    m_itemCount = SLAB_SIZE / m_itemSize;
    switch (nameEnum) {
    case GlobalVariables:
      m_itemCount = 1;
      break;
    case Variant:
    case Array:
    case SharedMap:
      m_itemCount = 128; // rarely used items belong to this group
      m_maxMultiplier = SLAB_SIZE / (m_itemSize * m_itemCount);
      ASSERT(m_maxMultiplier >= 1);
      break;
    case ObjectData:
      m_itemCount = calculate_item_count(m_itemSize);
      m_maxMultiplier = SLAB_SIZE / (m_itemSize * m_itemCount);
      ASSERT(m_maxMultiplier >= 1);
      break;
    case Bucket:
      m_itemCount *= 4; // we need lots of Buckets
      break;
    }
  }

  ASSERT(itemCount);
  ASSERT(itemSize);

  registerStats(&MemoryManager::TheMemoryManager()->getStats());
  ASSERT(m_stats);

  m_colMax = m_itemSize * m_itemCount;
  char *p = (char *)malloc(m_colMax);
  m_blocks.push_back(p);
  m_blockIndex[((int64)p) / m_colMax] = 0;
#ifdef USE_JEMALLOC
  // Cancel out jemalloc's accounting for this slab.
  m_stats->usage -= m_colMax;
#endif
  m_stats->alloc += m_colMax;
  if (m_stats->alloc > m_stats->peakAlloc) {
    m_stats->peakAlloc = m_stats->alloc;
  }

  if (nameEnum < 0) {
    m_name = "(unknown)";
  } else {
    static const char *TypeNames[] = {
#define SMART_ALLOCATOR_ENTRY(x) #x,
#include "smart_allocator.inc"
#undef SMART_ALLOCATOR_ENTRY
    };
    ASSERT(nameEnum < (int)(sizeof(TypeNames)/sizeof(TypeNames[0])));
    m_name = TypeNames[nameEnum];
  }

  MemoryManager::TheMemoryManager()->add(this);
}

SmartAllocatorImpl::~SmartAllocatorImpl() {
  unsigned int size = m_blocks.size();
  for (unsigned int i = m_backupBlocks.size(); i < size; i += m_multiplier) {
    free(m_blocks[i]);
  }
  size = m_backupBlocks.size();
  for (unsigned int i = 0; i < size; i++) {
    free(m_blocks[i]);
    free(m_backupBlocks[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////
// alloc/dealloc helpers

__attribute__ ((section (".text.hot")))
void *SmartAllocatorImpl::alloc() {
#ifdef SMART_ALLOCATOR_STACKTRACE
  {
    Lock lock(s_st_mutex);
    bool enabled = StackTrace::Enabled;
    StackTrace::Enabled = true;
    s_st_allocs.operator[](m_freelist.back());
    StackTrace::Enabled = enabled;
  }
#endif
  ASSERT(m_stats);
  // Just update the usage, while the peakUsage is maintained by
  // FrameInjection.
  m_stats->usage += m_itemSize;
  if (m_freelist.size() > 0) {
    // Fast path
    void *ret = m_freelist.back();
    m_freelist.pop_back();
    return ret;
  } else if (m_col < m_colMax) {
    char *ret = m_blocks[m_row] + m_col;
    m_col += m_itemSize;
    return ret;
  }
  // Slow path
  return allocHelper();
}

void *SmartAllocatorImpl::allocHelper() {
  ASSERT(m_col >= m_colMax);
  if (m_allocatedBlocks == 0) {
    // used up the last batch
    ASSERT((m_blocks.size() - m_backupBlocks.size()) % m_multiplier == 0);
    size_t size = m_colMax * m_multiplier;
    char *p = (char *)malloc(size);
    m_blocks.push_back(p);
    m_blockIndex[((int64)p) / m_colMax] = m_blocks.size() - 1;
#ifdef USE_JEMALLOC
    // Cancel out jemalloc's accounting for this slab.
    m_stats->usage -= size;
#endif
    m_allocatedBlocks = m_multiplier - 1;

    m_stats->alloc += size;
    if (m_stats->alloc > m_stats->peakAlloc) {
      m_stats->peakAlloc = m_stats->alloc;
    }
  } else {
    // still have some blocks left from the last batch
    char *p = m_blocks.back() + m_colMax;
    m_blocks.push_back(p);
    m_blockIndex[((int64)p) / m_colMax] = m_blocks.size() - 1;
    m_allocatedBlocks--;
  }

  m_row++;
  ASSERT(m_row == (int)m_blocks.size() - 1);
  ASSERT(m_col == m_colMax);
  m_col = 0;

  char *ret = m_blocks[m_row] + m_col;
  m_col += m_itemSize;
  return ret;
}

bool SmartAllocatorImpl::isValid(void *obj) const {
  if (obj) {
#ifdef DETECT_DOUBLE_FREE
    FreeList *fl = const_cast<FreeList *>(&m_freelist);
    for (FreeList::iterator it = fl->begin(); it != fl->end(); ++it) {
      void *p = *it;
      if (p == obj) return false;
    }
#endif

    // Check obj is indeed from a slab.
    int idx = findIndex(m_blocks, m_blockIndex, (int64)obj, m_colMax);
    char *block = m_blocks[idx];
    return obj >= block && obj < block + m_colMax &&
           (((char*)obj - block) % m_itemSize) == 0;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// SmartAllocatorManager methods

/**
 * When we restore, destination's size is always bigger, therefore, we can
 * fully restore destination's old contents WITHOUT malloc-ing new memory. This
 * is crucial in understanding why internal pointers between fixed size objects
 * are able to get maintained during the process.
 */
void SmartAllocatorImpl::copyMemoryBlocks(std::vector<char *> &dest,
                                          const std::vector<char *> &src,
                                          int lastCol,
                                          int lastBlockSize) {
  int sizeDest = dest.size();
  int sizeSrc = src.size();
  ASSERT(lastCol <= lastBlockSize && lastBlockSize <= m_colMax);
  ASSERT(sizeSrc > 0);
  if (sizeDest < sizeSrc) {
    dest.resize(sizeSrc);
    for (int i = sizeDest; i < sizeSrc - 1; i++) {
      dest[i] = (char *)malloc(m_colMax);
    }
    dest[sizeSrc - 1] = (char *)malloc(lastBlockSize);
  } else if (sizeDest > sizeSrc) {
    for (int i = sizeSrc; i < sizeDest; i++) {
      free(dest[i]);
    }
    dest.resize(sizeSrc);
  }
  for (int i = 0; i < sizeSrc - 1; i++) {
    memcpy(dest[i], src[i], m_colMax);
  }
  memcpy(dest[sizeSrc - 1], src[sizeSrc - 1], lastCol);
}

int SmartAllocatorImpl::calculateObjects(LinearAllocator &allocator,
                                         int &size) {
  int count = 0;
  int oldSize = size;
  if (m_flag & (NeedRestore | NeedRestoreOnce)) {
    m_iter.clear();
    for (m_iter.begin(); m_iter.get(); m_iter.next()) {
      if (calculate(m_iter.get(), size)) {
        ++count;
      }
    }
    ++count; // we need NULL termination for each type
  }
  m_linearSize = size - oldSize;
  m_linearCount = count;
  return count;
}

void SmartAllocatorImpl::backupObjects(LinearAllocator &allocator) {
  // backup internal pointers
  m_rowChecked = m_row;
  m_colChecked = m_col;

  // backup fixed size memory
  copyMemoryBlocks(m_backupBlocks, m_blocks, m_col, m_col);

  // backup free list
  m_backupFreelist.copy(m_freelist);

  // backup variable sized memory
  if (m_flag & (NeedRestore | NeedRestoreOnce)) {
    for (m_iter.begin(); m_iter.get(); m_iter.next()) {
      int size = 0;
      if (calculate(m_iter.get(), size)) {
        allocator.backup(m_iter.get());
        backup(m_iter.get(), allocator);
      }
    }
    allocator.backup((void*)NULL); // indicating end of this type
    m_iter.clear();
  }
}

void SmartAllocatorImpl::rollbackObjects(LinearAllocator &allocator) {
  // sweep dangling objects
  if (m_flag & (NeedRestore | NeedRestoreOnce | NeedSweep)) {
    m_iter.clear();
    for (m_iter.begin(); m_iter.get(); m_iter.next()) {
      sweep(m_iter.get());
    }
    m_iter.clear();
  }

  // restore internal pointers
  m_row = m_rowChecked;
  m_col = m_colChecked;

  // restore freelist if back'ed up
  if (!(m_flag & RestoreDisabled)) {
    m_freelist.copy(m_backupFreelist);
  } else {
    m_freelist.clear();
  }

  // restore fixed size memory
  ASSERT(m_blocks.size() >= m_backupBlocks.size());
  ASSERT(m_freelist.capacity() >= m_backupFreelist.capacity());

  // update the target multiplier
  m_targetMultiplier += m_blocks.size() - m_backupBlocks.size();
  m_targetMultiplier >>= 1;

  int newMultiplier = m_multiplier;
  // adjust the multiplier, but not too fast
  if (m_multiplier < m_targetMultiplier) {
    newMultiplier <<= 1;
    if (newMultiplier > m_maxMultiplier) newMultiplier = m_maxMultiplier;
  } else if ((m_multiplier >> 1) >= m_targetMultiplier && m_multiplier >= 2) {
    newMultiplier >>= 1;
  }

  m_blockIndex.clear();
  if (m_backupBlocks.empty()) {
    // this happens when SmartAllocator was created after checkpoint was taken
    ASSERT(m_row == 0);
    ASSERT(m_col == 0);
    ASSERT(m_freelist.size() == 0);
    for (unsigned int i = m_multiplier; i < m_blocks.size();
         i += m_multiplier) {
      free(m_blocks[i]);
    }
    m_blocks.resize(1);
    if (m_multiplier != newMultiplier) {
      char *p = (char *)realloc(m_blocks[0], m_colMax * newMultiplier);
      m_blocks[0] = p;
    }
    m_blockIndex[((int64)m_blocks[0]) / m_colMax] = 0;

    m_multiplier = newMultiplier;
    m_allocatedBlocks = m_multiplier - 1;
  } else {
    for (unsigned int i = m_backupBlocks.size(); i < m_blocks.size();
         i += m_multiplier) {
      free(m_blocks[i]);
    }
    m_blocks.resize(m_backupBlocks.size());
    copyMemoryBlocks(m_blocks, m_backupBlocks, m_colChecked, m_colMax);
    for (unsigned i = 0; i < m_blocks.size(); i++) {
      m_blockIndex[((int64)m_blocks[i]) / m_colMax] = i;
    }

    // restore variable sized memory
    if (((m_flag & RestoreDisabled) == 0)) {
      if (m_linearized) {
        allocator.advance(m_linearSize, m_linearCount);
      } else if (m_flag & (NeedRestore | NeedRestoreOnce)) {
        void *p;
        const char *data;
        while ((p = allocator.restore(data)) != NULL) {
          restore(p, data);
        }
        if (m_flag & NeedRestoreOnce) {
          copyMemoryBlocks(m_backupBlocks, m_blocks,
                           m_colChecked, m_colChecked);
          m_linearized = true;
        }
      }
    }

    m_multiplier = newMultiplier;
    m_allocatedBlocks = 0;
  }
}

void SmartAllocatorImpl::logStats() {
  int allocated = m_itemCount * m_row + (m_col / m_itemSize);
  int freed = m_freelist.size();

  string key = string("mem.") + m_name + "." +
    lexical_cast<string>(m_itemSize);
  ServerStats::Log(key + ".alloc", allocated);
  ServerStats::Log(key + ".freed", freed);
}

void SmartAllocatorImpl::checkMemory(bool detailed) {
  int allocated = m_itemCount * m_row + (m_col / m_itemSize);
  int freed = m_freelist.size();
  printf("%16s (%6d bytes %6d x %3d): %s %8d alloc %8d free\n",
         m_name, m_itemSize, m_itemCount, (m_row + 1),
         freed != allocated ? "bad" : "ok ", allocated, freed);

  if (detailed) {
    std::set<void *> freelist;
    int index = 0;
#define MAX_REPORT 10
    int count = MAX_REPORT;
    for (FreeList::iterator it = m_freelist.begin();
         it != m_freelist.end(); ++it) {
      void *p = *it;
      if (freelist.find(p) != freelist.end()) {
        if (--count == -1) {
          printf("stopped reporting more than %d double-freed items\n",
                 MAX_REPORT);
        } else if (count > 0) {
          printf("Double-freed Item %d:\n", ++index);
          dump(p);
#ifdef SMART_ALLOCATOR_STACKTRACE
          Lock lock(s_st_mutex);
          printf("%s\n", s_st_deallocs[p].toString().c_str());
#endif
        }
      } else {
        freelist.insert(p);
      }
    }

    index = 0;
    count = MAX_REPORT;
    for (int i = 0; i <= m_row; i++) {
      int jmax = m_colMax;
      if (i == m_row) jmax = m_col;
      for (int j = 0; j < jmax; j += m_itemSize) {
        void *p = m_blocks[i] + j;
        if (freelist.find(p) == freelist.end()) {
          if (--count == -1) {
            printf("stopped reporting more than %d leaked items\n",
                   MAX_REPORT);
          } else if (count > 0) {
            printf("Leaked Item at {%d:%d} %d:\n", i, j/m_itemSize, ++index);
            dump(p);
#ifdef SMART_ALLOCATOR_STACKTRACE
            Lock lock(s_st_mutex);
            printf("%s\n", s_st_allocs[p].toString().c_str());
#endif
          }
        } else {
          freelist.erase(p);
        }
      }
    }

    count = MAX_REPORT;
    for (std::set<void *>::const_iterator iter = freelist.begin();
         iter != freelist.end(); ++iter) {
      if (--count == -1) {
        printf("stopped reporting more than %d invalid items\n",
               MAX_REPORT);
      } else if (count > 0) {
        void *p = *iter;
        printf("Invalid Item %p:\n", p);
#ifdef SMART_ALLOCATOR_STACKTRACE
        Lock lock(s_st_mutex);
        printf("%s\n", s_st_deallocs[p].toString().c_str());
        ASSERT(s_st_allocs.find(p) == s_st_allocs.end());
#endif
      }
    }
  }
}

void SmartAllocatorImpl::prepareIterator(FreeMap &freeMap) {
  ASSERT(freeMap.empty());
  freeMap.resize(m_blocks.size() * m_itemCount);
  for (FreeList::iterator it = m_freelist.begin(); it != m_freelist.end();
       ++it) {
    int64 freed = (int64)(*it);
    int idx = findIndex(m_blocks, m_blockIndex, freed, m_colMax);

    // no double free!
    ASSERT(!freeMap.test(idx * m_itemCount +
                         (freed - (int64)m_blocks[idx]) / m_itemSize));
    freeMap.set(idx * m_itemCount +
                (freed - (int64)m_blocks[idx]) / m_itemSize);
  }
}

///////////////////////////////////////////////////////////////////////////////
// PointerIterator

SmartAllocatorImpl::
PointerIterator::PointerIterator(SmartAllocatorImpl *allocator)
  : m_allocator(allocator), m_px(NULL), m_prepared(false),
    m_curRow(0), m_offset(0), m_curFree(0) {
  ASSERT(allocator);
  m_itemSize = allocator->getItemSize();
  m_itemCount = allocator->getItemCount();
}

void SmartAllocatorImpl::PointerIterator::clear() {
  m_freeMap.clear();
  m_prepared = false;
  m_px = NULL;
  m_curRow = 0;
  m_offset = 0;
  m_curFree = 0;
}

void SmartAllocatorImpl::PointerIterator::begin() {
  if (!m_prepared) {
    m_allocator->prepareIterator(m_freeMap);
    m_prepared = true;
  }

  m_px = NULL;
  m_curRow = 0;
  m_offset = 0;
  m_curFree = 0;
  while (search());
}

void SmartAllocatorImpl::PointerIterator::next() {
  ASSERT(m_px);
  m_offset += m_itemSize;
  m_curFree++;
  while (search());
}

bool SmartAllocatorImpl::PointerIterator::search() {
  if (m_curRow == m_allocator->m_row && m_offset >= m_allocator->m_col) {
    m_px = NULL;
    return false;
  }
  if (m_offset >= m_allocator->m_colMax) {
    m_curRow++;
    m_offset = 0;
  } else {
    m_px = m_allocator->m_blocks[m_curRow] + m_offset;
    if (!m_freeMap.test(m_curFree)) {
      return false;
    }
    m_offset += m_itemSize;
    m_curFree++;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// ObjectAllocator classes

ObjectAllocatorBase::ObjectAllocatorBase(int itemSize)
  : SmartAllocatorImpl(SmartAllocatorImpl::ObjectData, -1, itemSize,
                       SmartAllocatorImpl::NoCallbacks) { }

int ObjectAllocatorBase::calculate(void *p, int &size) {
  return false;
}

void ObjectAllocatorBase::backup(void *p, LinearAllocator &allocator) {
  // do nothing
}

void ObjectAllocatorBase::restore(void *p, const char *&data) {
  // do nothing
}

void ObjectAllocatorBase::sweep(void *p) {
  // do nothing
}

void ObjectAllocatorBase::dump(void *p) {
  printf("%p", p);
}

///////////////////////////////////////////////////////////////////////////////
}
