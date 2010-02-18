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

#include <cpp/base/memory/smart_allocator.h>
#include <cpp/base/memory/memory_manager.h>
#include <cpp/base/resource_data.h>
#include <cpp/base/server/server_stats.h>
#include <cpp/base/runtime_option.h>
#include <util/logger.h>

using namespace std;
using namespace boost;

#define MAX_OBJECT_COUNT_PER_SLAB 32
#define SLAB_SIZE (128 * 1024)

namespace HPHP {

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

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

SmartAllocatorImpl::SmartAllocatorImpl(int nameEnum, int itemCount,
                                       int itemSize, int flag)
  : m_itemCount(itemCount), m_itemSize(itemSize), m_flag(flag),
    m_row(0), m_col(0), m_pos(-1),
    m_rowChecked(0), m_colChecked(0), m_posChecked(-1), m_linearSize(0),
    m_linearCount(0), m_iter(this), m_dealloc(true), m_linearized(false),
    m_stats(NULL) {

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
      break;
    case ObjectData:
      m_itemCount = calculate_item_count(m_itemSize);
      break;
    case Bucket:
      m_itemCount *= 4; //  we need lots of Buckets
      break;
    }
  }

  ASSERT(itemCount);
  ASSERT(itemSize);

  m_colMax = m_itemSize * m_itemCount;
  m_blocks.push_back((char *)malloc(m_colMax));
  m_freelist.resize(m_itemCount);
  if (m_stats) {
    m_stats->alloc += m_colMax;
    if (m_stats->alloc > m_stats->peakAlloc) {
      m_stats->peakAlloc = m_stats->alloc;
    }
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
  for (unsigned int i = 0; i < size; i++) {
    free(m_blocks[i]);
  }
  size = m_backupBlocks.size();
  for (unsigned int i = 0; i < size; i++) {
    free(m_backupBlocks[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////
// two most important methods

void *SmartAllocatorImpl::alloc() {
  if (m_stats) {
    m_stats->usage += m_itemSize;
    if (m_stats->usage > m_stats->peakUsage) {
      int64 prevPeakUsage = m_stats->peakUsage;
      m_stats->peakUsage = m_stats->usage;
      if (RuntimeOption::RequestMemoryMaxBytes > 0 &&
          m_stats->peakUsage > RuntimeOption::RequestMemoryMaxBytes &&
          prevPeakUsage <= RuntimeOption::RequestMemoryMaxBytes) {
        throw FatalErrorException("request has exceeded memory limit");
      }
    }
  }

  if (m_pos > -1) {
#ifdef SMART_ALLOCATOR_STACKTRACE
    m_st_allocs.operator[](m_freelist[m_pos]);
#endif
    return m_freelist[m_pos--];
  }
  if (m_col >= m_colMax) {
    m_blocks.push_back((char *)malloc(m_colMax));
    if (m_stats) {
      m_stats->alloc += m_colMax;
      if (m_stats->alloc > m_stats->peakAlloc) {
        m_stats->peakAlloc = m_stats->alloc;
      }
    }

    m_row++;
    ASSERT(m_row == (int)m_blocks.size() - 1);
    ASSERT(m_col == m_colMax);
    m_col = 0;
    m_freelist.resize(m_freelist.size() + m_itemCount);
    ASSERT((int)m_freelist.size() == (m_row + 1) * m_itemCount);
  }
  char *ret = m_blocks[m_row] + m_col;
  m_col += m_itemSize;
#ifdef SMART_ALLOCATOR_STACKTRACE
  m_st_allocs.operator[](ret);
#endif
  return ret;
}

void SmartAllocatorImpl::dealloc(void *obj) {
  if (obj) {
    ASSERT(isValid(obj));
    ASSERT(m_pos < (int)m_freelist.size() - 1);
    m_freelist[++m_pos] = obj;
#ifdef SMART_ALLOCATOR_STACKTRACE
    m_st_deallocs.operator[](obj);
#endif

    if (m_stats) {
      m_stats->usage -= m_itemSize;
    }
  }
}

bool SmartAllocatorImpl::isValid(void *obj) const {
  if (obj) {
    unsigned int size = m_blocks.size();
    for (unsigned int i = 0; i < size; i++) {
      char *block = m_blocks[i];
      if (obj >= block && obj < block + m_colMax &&
          (((char*)obj - block) % m_itemSize) == 0) {
        return true;
      }
    }
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
  m_posChecked = m_pos;

  // backup fixed size memory
  copyMemoryBlocks(m_backupBlocks, m_blocks, m_col, m_col);

  // backup free list
  m_backupFreelist = m_freelist;

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
  m_dealloc = true;

  // sweep dangling objects
  if (m_flag & (NeedRestore | NeedRestoreOnce | NeedSweep)) {
    m_iter.clear();
    for (m_iter.begin(); m_iter.get(); m_iter.next()) {
      sweep(m_iter.get());
    }
    m_iter.clear();
  }
  Sweepable::SweepAll();

  // restore internal pointers
  m_row = m_rowChecked;
  m_col = m_colChecked;
  m_pos = m_posChecked;

  // restore freelist if back'ed up
  if (!(m_flag & RestoreDisabled)) {
    m_freelist = m_backupFreelist;
  } else {
    m_freelist.resize(m_itemCount);
  }

  // restore fixed size memory
  ASSERT(m_blocks.size() >= m_backupBlocks.size());
  ASSERT(m_freelist.size() >= m_backupFreelist.size());
  if (m_backupBlocks.empty()) {
    // this happens when SmartAllocator was created after checkpoint was taken
    ASSERT(m_row == 0);
    ASSERT(m_col == 0);
    ASSERT(m_pos == -1);
    for (unsigned int i = 1; i < m_blocks.size(); i++) {
      free(m_blocks[i]);
    }
    m_blocks.resize(1);
  } else {
    copyMemoryBlocks(m_blocks, m_backupBlocks, m_colChecked, m_colMax);

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
  }
}

void SmartAllocatorImpl::logStats() {
  int allocated = m_itemCount * m_row + (m_col / m_itemSize);
  int freed = m_pos + 1;

  string key = string("mem.") + m_name + "." +
    lexical_cast<string>(m_itemSize);
  ServerStats::Log(key + ".alloc", allocated);
  ServerStats::Log(key + ".freed", freed);
}

void SmartAllocatorImpl::checkMemory(bool detailed) {
  int allocated = m_itemCount * m_row + (m_col / m_itemSize);
  int freed = m_pos + 1;
  printf("%16s (%6d bytes %6d x %3d): %s %8d alloc %8d free\n",
         m_name, m_itemSize, m_itemCount, (m_row + 1),
         freed != allocated ? "bad" : "ok ", allocated, freed);

  if (detailed) {
    std::set<void *> freelist;
    int index = 0;
#define MAX_REPORT 10
    int count = MAX_REPORT;
    for (int i = 0; i <= m_pos; i++) {
      void *p = m_freelist[i];
      if (freelist.find(p) != freelist.end()) {
        if (--count == -1) {
          printf("stopped reporting more than %d double-freed items\n",
		 MAX_REPORT);
        } else if (count > 0) {
	  printf("Double-freed Item %d:\n", ++index);
	  dump(p);
#ifdef SMART_ALLOCATOR_STACKTRACE
          printf("%s\n", m_st_deallocs[p].toString().c_str());
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
	    printf("%s\n", m_st_allocs[p].toString().c_str());
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
	printf("%s\n", m_st_deallocs[p].toString().c_str());
        ASSERT(m_st_allocs.find(p) == m_st_allocs.end());
#endif
      }
    }
  }
}

void SmartAllocatorImpl::prepareIterator(std::set<char*> &freelistSet,
                                         std::map<char*, int> &blockInfoMap) {
  ASSERT(freelistSet.empty());
  for (int i = 0; i <= m_pos; i++) {
    char *p = (char*)m_freelist[i];
    ASSERT(freelistSet.find(p) == freelistSet.end());
    freelistSet.insert(p);
  }

  ASSERT(blockInfoMap.empty());
  for (int i = 0; i < m_row; i++) {
    blockInfoMap[m_blocks[i]] = m_colMax;
  }
  blockInfoMap[m_blocks[m_row]] = m_col;
}

///////////////////////////////////////////////////////////////////////////////
// PointerIterator

SmartAllocatorImpl::
PointerIterator::PointerIterator(SmartAllocatorImpl *allocator)
  : m_allocator(allocator), m_px(NULL), m_prepared(false), m_offset(0),
    m_curFree(NULL) {
  ASSERT(allocator);
  m_itemSize = allocator->getItemSize();
}

void SmartAllocatorImpl::PointerIterator::clear() {
  m_freelistSet.clear();
  m_blockInfoMap.clear();
  m_prepared = false;
  m_px = NULL;
  m_offset = 0;
  m_curFree = NULL;
}

void SmartAllocatorImpl::PointerIterator::begin() {
  if (!m_prepared) {
    m_allocator->prepareIterator(m_freelistSet, m_blockInfoMap);
    m_prepared = true;
  }

  m_iterFreelist = m_freelistSet.begin();
  if (m_iterFreelist == m_freelistSet.end()) {
    m_curFree = NULL;
  } else {
    m_curFree = *m_iterFreelist;
  }

  m_px = NULL;
  m_iterBlock = m_blockInfoMap.begin();
  if (m_iterBlock != m_blockInfoMap.end()) {
    m_offset = 0;
    while (search());
  }
}

void SmartAllocatorImpl::PointerIterator::next() {
  ASSERT(m_px);
  m_offset += m_itemSize;
  while (search());
}

bool SmartAllocatorImpl::PointerIterator::search() {
  if (m_offset >= m_iterBlock->second) {
    if (++m_iterBlock == m_blockInfoMap.end()) {
      m_px = NULL;
      return false;
    }
    m_offset = 0;
  } else {
    m_px = m_iterBlock->first + m_offset;
    if (m_px < m_curFree || m_curFree == NULL) {
      return false;
    }

    ASSERT(m_px == m_curFree);
    if (++m_iterFreelist == m_freelistSet.end()) {
      m_curFree = NULL;
    } else {
      m_curFree = *m_iterFreelist;
    }
    m_offset += m_itemSize;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// ObjectAllocator classes

ObjectAllocatorBase::ObjectAllocatorBase(int itemSize)
  : SmartAllocatorImpl(SmartAllocatorImpl::ObjectData,
                       calculate_item_count(itemSize), itemSize,
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

int ObjectAllocatorWrapper::getAllocatorSeqno(int size) {
  int r = 0;
  int s = sizeof(ObjectData);
  while (s < size) {
    r++;
    s += (s >> 1);
    s = ALIGN_WORD(s);
  }
  return r;
}

///////////////////////////////////////////////////////////////////////////////
}
