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

namespace HPHP {

TRACE_SET_MOD(debuggerflow);

//////////////////////////////////////////////////////////////////////////

struct PCFilter::PtrMapNode {
  void **m_entries;
  void clearImpl(unsigned short bits);
};

void PCFilter::PtrMapNode::clearImpl(unsigned short bits) {
  // clear all the sub levels and mark all slots NULL
  if (bits <= PTRMAP_LEVEL_BITS) {
    assert(bits == PTRMAP_LEVEL_BITS);
    // On bottom level, pointers are not PtrMapNode*
    memset(m_entries, 0, sizeof(void*) * PTRMAP_LEVEL_ENTRIES);
    return;
  }
  for (int i = 0; i < PTRMAP_LEVEL_ENTRIES; i++) {
    if (m_entries[i]) {
      ((PCFilter::PtrMapNode*)m_entries[i])->clearImpl(bits -
                                                       PTRMAP_LEVEL_BITS);
      free(((PCFilter::PtrMapNode*)m_entries[i])->m_entries);
      free(m_entries[i]);
      m_entries[i] = nullptr;
    }
  }
}

PCFilter::PtrMapNode* PCFilter::PtrMap::MakeNode() {
  PtrMapNode* node = (PtrMapNode*)malloc(sizeof(PtrMapNode));
  node->m_entries =
    (void**)calloc(1, PTRMAP_LEVEL_ENTRIES * sizeof(void*));
  return node;
}

PCFilter::PtrMap::~PtrMap() {
  clear();
  free(m_root->m_entries);
  free(m_root);
}

void* PCFilter::PtrMap::getPointer(void* ptr) {
  PtrMapNode* current = m_root;
  unsigned short cursor = PTRMAP_PTR_SIZE;
  while (current && cursor) {
    cursor -= PTRMAP_LEVEL_BITS;
    unsigned long index = ((PTRMAP_LEVEL_MASK << cursor) & (unsigned long)ptr)
                          >> cursor;
    assert(index < PTRMAP_LEVEL_ENTRIES);
    current = (PtrMapNode*)(current->m_entries[index]);
  }
  return (void*)current;
}

void PCFilter::PtrMap::setPointer(void* ptr, void* val) {
  PtrMapNode* current = m_root;
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
      current->m_entries[index] = (void*) MakeNode();
    }
    current = (PtrMapNode*)(current->m_entries[index]);
  }
}

void PCFilter::PtrMap::clear() {
  m_root->clearImpl(PTRMAP_PTR_SIZE);
}

// Adds a range of PCs to the filter given a collection of offset ranges.
// Omit PCs which have opcodes that don't pass the given opcode filter.
void PCFilter::addRanges(const Unit* unit, const OffsetRangeVec& offsets,
                         OpcodeFilter isOpcodeAllowed) {
  for (auto range = offsets.cbegin(); range != offsets.cend(); ++range) {
    TRACE(3, "\toffsets [%d, %d)\n", range->m_base, range->m_past);
    for (PC pc = unit->at(range->m_base); pc < unit->at(range->m_past);
         pc += instrLen((Op*)pc)) {
      if (isOpcodeAllowed(*reinterpret_cast<const Op*>(pc))) {
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
    TRACE(3, "\toffsets [%d, %d) (remove)\n", range->m_base, range->m_past);
    for (PC pc = unit->at(range->m_base); pc < unit->at(range->m_past);
         pc += instrLen((Op*) pc)) {
      if (isOpcodeAllowed(*reinterpret_cast<const Op*>(pc))) {
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
