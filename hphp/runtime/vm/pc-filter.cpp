/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/vm/pc-filter.h"

#include "hphp/runtime/vm/hhbc-codec.h"

namespace HPHP {

TRACE_SET_MOD(debuggerflow);

//////////////////////////////////////////////////////////////////////////

struct PCFilter::PtrMapNode {
  void **m_entries;
  void clearImpl(unsigned short bits);
};

void PCFilter::PtrMapNode::clearImpl(unsigned short bits) {
  static_assert(sizeof(PCFilter::PtrMapNode) == sizeof(PCFilter::PtrMap) &&
                sizeof(PCFilter::PtrMapNode) == sizeof(void*),
                "PtrMapNode and PtrMap must hold a single pointer");

  // clear all the sub levels and mark all slots NULL
  if (bits <= PTRMAP_LEVEL_BITS) {
    assert(bits == PTRMAP_LEVEL_BITS);
    return;
  }
  for (int i = 0; i < PTRMAP_LEVEL_ENTRIES; i++) {
    if (m_entries[i]) {
      ((PCFilter::PtrMapNode*)&m_entries[i])->clearImpl(bits -
                                                        PTRMAP_LEVEL_BITS);
      free(m_entries[i]);
      m_entries[i] = nullptr;
    }
  }
}

static void* MakeNode() {
  return calloc(PTRMAP_LEVEL_ENTRIES, sizeof(void*));
}

void* PCFilter::PtrMap::getPointerImpl(void* ptr) const {
  auto current = (PtrMapNode*)&m_root;
  assert(current->m_entries);
  unsigned short cursor = PTRMAP_PTR_SIZE;
  do {
    cursor -= PTRMAP_LEVEL_BITS;
    unsigned long index = ((PTRMAP_LEVEL_MASK << cursor) & (unsigned long)ptr)
                          >> cursor;
    assert(index < PTRMAP_LEVEL_ENTRIES);
    current = (PtrMapNode*)&current->m_entries[index];
  } while (current->m_entries && cursor);
  return current->m_entries;
}

void PCFilter::PtrMap::setPointer(void* ptr, void* val) {
  PtrMapNode* current = (PtrMapNode*)&m_root;
  if (!current->m_entries) {
    if (!val) return;
    m_root = MakeNode();
  }
  assert(current->m_entries == m_root);

  unsigned short cursor = PTRMAP_PTR_SIZE;
  while (true) {
    cursor -= PTRMAP_LEVEL_BITS;
    unsigned long index = ((PTRMAP_LEVEL_MASK << cursor) & (unsigned long)ptr)
                          >> cursor;
    assert(index < PTRMAP_LEVEL_ENTRIES);
    if (!cursor) {
      current->m_entries[index] = val;
      break;
    }
    if (!current->m_entries[index])  {
      current->m_entries[index] = MakeNode();
    }
    current = (PtrMapNode*)&current->m_entries[index];
  }
}

void PCFilter::PtrMap::clear() {
  if (m_root) {
    auto current = (PtrMapNode*)&m_root;
    current->clearImpl(PTRMAP_PTR_SIZE);
    free(m_root);
    m_root = nullptr;
  }
}

// Adds a range of PCs to the filter given a collection of offset ranges.
// Omit PCs which have opcodes that don't pass the given opcode filter.
void PCFilter::addRanges(const Unit* unit, const OffsetRangeVec& offsets,
                         OpcodeFilter isOpcodeAllowed) {
  for (auto range = offsets.cbegin(); range != offsets.cend(); ++range) {
    TRACE(3, "\toffsets [%d, %d)\n", range->base, range->past);
    for (PC pc = unit->at(range->base); pc < unit->at(range->past);
         pc += instrLen(pc)) {
      if (isOpcodeAllowed(peek_op(pc))) {
        TRACE(3, "\t\tpc %p\n", pc);
        addPC(pc);
      } else {
        TRACE(3, "\t\tpc %p -- skipping (offset %d)\n", pc,
              unit->offsetOf(pc));
      }
    }
  }
}

// Removes a range of PCs to the filter given a collection of offset ranges.
// Omit PCs which have opcodes that don't pass the given opcode filter.
void PCFilter::removeRanges(const Unit* unit, const OffsetRangeVec& offsets,
                            OpcodeFilter isOpcodeAllowed) {
  for (auto range = offsets.cbegin(); range != offsets.cend(); ++range) {
    TRACE(3, "\toffsets [%d, %d) (remove)\n", range->base, range->past);
    for (PC pc = unit->at(range->base); pc < unit->at(range->past);
         pc += instrLen(pc)) {
      if (isOpcodeAllowed(peek_op(pc))) {
        TRACE(3, "\t\tpc %p (remove)\n", pc);
        removePC(pc);
      } else {
        TRACE(3, "\t\tpc %p -- skipping (offset %d) (remove)\n", pc,
              unit->offsetOf(pc));
      }
    }
  }
}

void PCFilter::removeOffset(const Unit* unit, Offset offset) {
  removePC(unit->at(offset));
}

///////////////////////////////////////////////////////////////////////////////
}
