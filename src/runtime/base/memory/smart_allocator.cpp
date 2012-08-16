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

/*
 * Enabling these will prevent us from allocating out of the free list
 * and cause deallocated objects to be filled with garbage.  This is
 * intended for detecting data that is freed too eagerly.
 */
#if defined(SMART_ALLOCATOR_DEBUG_FREE) && !defined(DETECT_DOUBLE_FREE)
# define DETECT_DOUBLE_FREE
#endif

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

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

SmartAllocatorImpl::SmartAllocatorImpl(int nameEnum, int itemCount,
                                       int itemSize, int flag)
  : m_stats(NULL)
  , m_itemSize(roundup(itemSize))
  , m_row(0)
  , m_nameEnum(Name(nameEnum))
  , m_itemCount(itemCount)
  , m_flag(flag)
  , m_allocatedBlocks(0)
  , m_multiplier(1)
  , m_maxMultiplier(1)
  , m_targetMultiplier(1)
{
  // automatically pick a good per slab item count
  if (m_itemCount <= 0) {
    m_itemCount = SLAB_SIZE / m_itemSize;
    switch (nameEnum) {
    case GlobalVariables:
      m_itemCount = 1;
      break;
    case RefData:
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
    default:
      break;
    }
  }

  ASSERT(itemCount);
  ASSERT(itemSize);

  MemoryManager* mm = MemoryManager::TheMemoryManager().getNoCheck();
  m_stats = &mm->getStats();

  m_colMax = m_itemSize * m_itemCount;
  char *p = (char *)malloc(m_colMax);
  m_next = p;
  m_limit = p + m_colMax;
  m_blocks.push_back(p);
  m_blockIndex[((int64)p) / m_colMax] = 0;
  // Cancel out jemalloc's accounting for this slab.
  JEMALLOC_STATS_ADJUST(m_stats, m_colMax);
  m_stats->alloc += m_colMax;
  if (m_stats->alloc > m_stats->peakAlloc) {
    m_stats->peakAlloc = m_stats->alloc;
  }

  if (nameEnum < 0) {
    m_name = "(unknown)";
  } else {
    static const char *TypeNames[] = {
#define SMART_ALLOCATOR_ENTRY(x) #x,
#include "runtime/base/memory/smart_allocator.inc_gen"
#undef SMART_ALLOCATOR_ENTRY
    };
    ASSERT(nameEnum < (int)(sizeof(TypeNames)/sizeof(TypeNames[0])));
    m_name = TypeNames[nameEnum];
  }

  mm->add(this);
}

SmartAllocatorImpl::~SmartAllocatorImpl() {
  unsigned int size = m_blocks.size();
  for (unsigned int i = 0; i < size; i += m_multiplier) {
    free(m_blocks[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////
// alloc/dealloc helpers

HOT_FUNC
void *SmartAllocatorImpl::alloc(size_t nbytes) {
  ASSERT(m_stats && nbytes == size_t(m_itemSize));
  ASSERT(m_next && m_next <= m_limit);
  MemoryUsageStats* stats = m_stats;
  // Just update the usage, while the peakUsage is maintained by
  // FrameInjection.
  int64 usage = stats->usage + nbytes;
  stats->usage = usage;
  if (hhvm && UNLIKELY(usage > stats->maxBytes)) {
    // It's possible that this simplified check will trip later than
    // it should in a perfect world but it's cheaper than a full call
    // to refreshStats on every alloc().
    statsHelper();
  }
#ifndef SMART_ALLOCATOR_DEBUG_FREE
  void* freelist_value = m_freelist.maybePop();
  if (LIKELY(freelist_value != NULL)) return freelist_value;
#endif
  char* p = m_next;
  if (LIKELY(p + nbytes <= m_limit)) {
    m_next = p + nbytes;
    return p;
  }
  // Slow path
  return allocHelper();
}

void *SmartAllocatorImpl::allocHelper() {
  ASSERT(m_next == m_limit);
  if (m_allocatedBlocks == 0) {
    // used up the last batch
    ASSERT(m_blocks.size() % m_multiplier == 0);
    size_t size = m_colMax * m_multiplier;
    char *p = (char *)malloc(size);
    m_blocks.push_back(p);
    m_blockIndex[((int64)p) / m_colMax] = m_blocks.size() - 1;
    // Cancel out jemalloc's accounting for this slab.
    JEMALLOC_STATS_ADJUST(m_stats, size);
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
  char *p = m_blocks[m_row];
  m_next = p + m_itemSize;
  m_limit = p + m_colMax;
  return p;
}

// cold-path helper function, only called when request memory overflow
// is likely.
void SmartAllocatorImpl::statsHelper() {
  ASSERT(m_stats->maxBytes > 0);
  MemoryManager::TheMemoryManager()->refreshStats();
}

bool SmartAllocatorImpl::assertValidHelper(void *obj) const {
  if (obj) {
#ifdef DETECT_DOUBLE_FREE
    GarbageList *fl = const_cast<GarbageList *>(&m_freelist);
    for (GarbageList::iterator it = fl->begin(); it != fl->end(); ++it) {
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

bool SmartAllocatorImpl::isFromThisAllocator(void* p) const {
  for (size_t i = 0; i < m_blocks.size(); ++i) {
    if (p >= m_blocks[i] && p < m_blocks[i] + m_colMax) {
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// SmartAllocatorManager methods

HOT_FUNC
void SmartAllocatorImpl::rollbackObjects() {
  // sweep dangling objects
  if (m_flag & NeedSweep) {
    FreeMap freeMap;
    prepareFreeMap(freeMap);
    int max = m_colMax;
    int bitIndex = 0;
    for (unsigned int i = 0; i < m_blocks.size(); i++) {
      char *start = (char *)m_blocks[i];
      if (i == m_blocks.size() - 1) max = m_next - start;
      for (char *obj = start; obj < start + max;
           obj += m_itemSize, bitIndex++) {
        if (!freeMap.test(bitIndex)) {
          sweep(obj);
        }
      }
    }
  }

  m_freelist.clear();

  // update the target multiplier
  m_targetMultiplier += m_blocks.size();
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

  ASSERT(m_freelist.empty());
  for (unsigned int i = m_multiplier; i < m_blocks.size();
       i += m_multiplier) {
    free(m_blocks[i]);
  }
  m_blocks.resize(1);
  if (m_multiplier != newMultiplier) {
    // don't use realloc because we don't want to pay for its memcpy.
    free(m_blocks[0]);
    m_blocks[0] = (char*) malloc(m_colMax * newMultiplier);
  }
  m_blockIndex[((int64)m_blocks[0]) / m_colMax] = 0;
  m_multiplier = newMultiplier;

  // reset for new allocations
  m_allocatedBlocks = m_multiplier - 1;
  m_row = 0;
  m_next = m_blocks[0];
  m_limit = m_next + m_colMax;
}

void SmartAllocatorImpl::logStats() {
  int col = m_next - m_blocks[m_row];
  int allocated = m_itemCount * m_row + (col / m_itemSize);
  int freed = m_freelist.size();

  string key = string("mem.") + m_name + "." +
    lexical_cast<string>(m_itemSize);
  ServerStats::Log(key + ".alloc", allocated);
  ServerStats::Log(key + ".freed", freed);
}

void SmartAllocatorImpl::checkMemory(bool detailed) {
  int col = m_next - m_blocks[m_row];
  int allocated = m_itemCount * m_row + (col / m_itemSize);
  int freed = m_freelist.size();
  printf("%16s (%6d bytes %6d x %3d): %s %8d alloc %8d free\n",
         m_name, m_itemSize, m_itemCount, (m_row + 1),
         freed != allocated ? "bad" : "ok ", allocated, freed);

  if (detailed) {
    std::set<void *> freelist;
    int index = 0;
#define MAX_REPORT 10
    int count = MAX_REPORT;
    for (GarbageList::iterator it = m_freelist.begin();
         it != m_freelist.end(); ++it) {
      void *p = *it;
      if (freelist.find(p) != freelist.end()) {
        if (--count == -1) {
          printf("stopped reporting more than %d double-freed items\n",
                 MAX_REPORT);
        } else if (count > 0) {
          printf("Double-freed Item %d:\n", ++index);
          dump(p);
        }
      } else {
        freelist.insert(p);
      }
    }

    index = 0;
    count = MAX_REPORT;
    for (int i = 0; i <= m_row; i++) {
      int jmax = m_colMax;
      if (i == m_row) jmax = m_next - m_blocks[i];
      for (int j = 0; j < jmax; j += m_itemSize) {
        void *p = m_blocks[i] + j;
        if (freelist.find(p) == freelist.end()) {
          if (--count == -1) {
            printf("stopped reporting more than %d leaked items\n",
                   MAX_REPORT);
          } else if (count > 0) {
            printf("Leaked Item at {%d:%d} %d:\n", i, j/m_itemSize, ++index);
            dump(p);
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
      }
    }
  }
}

HOT_FUNC
void SmartAllocatorImpl::prepareFreeMap(FreeMap& freeMap) const {
  ASSERT(freeMap.empty());
  freeMap.resize(m_blocks.size() * m_itemCount);
  for (GarbageList::iterator it = m_freelist.begin(); it != m_freelist.end();
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

static bool is_object_alive(char* caddr) {
  return *reinterpret_cast<int*>(caddr + FAST_REFCOUNT_OFFSET) !=
    RefCountTombstoneValue;
}

static bool UNUSED is_iterable_type(SmartAllocatorImpl::Name type) {
  return type == SmartAllocatorImpl::Variant ||
         type == SmartAllocatorImpl::RefData ||
         type == SmartAllocatorImpl::ObjectData ||
         type == SmartAllocatorImpl::StringData ||
         type == SmartAllocatorImpl::HphpArray ||
         type == SmartAllocatorImpl::Array ||
         type == SmartAllocatorImpl::VectorArray ||
         type == SmartAllocatorImpl::ZendArray;
}

SmartAllocatorImpl::Iterator::Iterator(const SmartAllocatorImpl* sa)
  : m_sa(*sa)
  , m_row(0)
  , m_col(-m_sa.m_itemSize)
{
  ASSERT(hhvm);
  ASSERT(is_iterable_type(sa->getAllocatorType()));
  next();
}

void* SmartAllocatorImpl::Iterator::current() const {
  return m_row == -1 ? 0 : m_sa.m_blocks[m_row] + m_col;
}

void SmartAllocatorImpl::Iterator::next() {
  do {
    m_col += m_sa.m_itemSize;
    if (m_col >= m_sa.m_colMax) {
      ASSERT(m_col == m_sa.m_colMax);
      if (++m_row >= m_sa.m_row) {
        m_row = -1;
        return;
      }
      m_col = 0;
    }

    int col = m_sa.m_next - m_sa.m_blocks[m_sa.m_row];
    if (m_row == m_sa.m_row && m_col >= col) {
      m_row = -1;
      return;
    }
  } while (!is_object_alive(static_cast<char*>(current())));
}

///////////////////////////////////////////////////////////////////////////////
// ObjectAllocator classes

ObjectAllocatorBase::ObjectAllocatorBase(int itemSize)
  : SmartAllocatorImpl(SmartAllocatorImpl::ObjectData, -1, itemSize,
                       SmartAllocatorImpl::NoCallbacks) { }

void ObjectAllocatorBase::sweep(void *p) {
  // do nothing
}

void ObjectAllocatorBase::dump(void *p) {
  printf("%p", p);
}

///////////////////////////////////////////////////////////////////////////////
}
