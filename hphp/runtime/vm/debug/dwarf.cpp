/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/debug/dwarf.h"

#include <stdio.h>
#include "debug.h"
#if !defined(__APPLE__) && !defined(__FreeBSD__)
#include "hphp/runtime/vm/debug/elfwriter.h"
#endif
#include "hphp/runtime/vm/debug/gdb-jit.h"

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

using namespace HPHP::JIT;

namespace HPHP {
namespace Debug {

int g_dwarfCallback(
  LIBDWARF_CALLBACK_NAME_TYPE name, int size, Dwarf_Unsigned type,
  Dwarf_Unsigned flags, Dwarf_Unsigned link, Dwarf_Unsigned info,
  Dwarf_Unsigned *sect_name_index, Dwarf_Ptr handle, int *error) {
#if !defined(__APPLE__) && !defined(__FreeBSD__)
  ElfWriter *e = reinterpret_cast<ElfWriter *>(handle);
  return e->dwarfCallback(name, size, type, flags, link, info);
#else
  return 0;
#endif
}

void DwarfBuf::byte(uint8_t c) {
  m_buf.push_back(c);
}

void DwarfBuf::byte(int off, uint8_t c) {
  assert((size_t)off < m_buf.size());
  m_buf[off] = c;
}

void DwarfBuf::word(uint16_t w) {
  byte(w & 0xff);
  byte((w >> 8) & 0xff);
}

void DwarfBuf::word(int off, uint16_t w) {
  byte(off, (w & 0xff));
  byte(off + 1, ((w >> 8) & 0xff));
}

void DwarfBuf::dword(uint32_t d) {
  word(d & 0xffff);
  word((d >> 16) & 0xffff);
}

void DwarfBuf::dword(int off, uint32_t d) {
  word(off, (d & 0xffff));
  word(off + 2, ((d >> 16) & 0xffff));
}

void DwarfBuf::qword(uint64_t q) {
  dword(q & 0xffffffff);
  dword((q >> 32) & 0xfffffffff);
}

void DwarfBuf::clear() {
  m_buf.clear();
}

int DwarfBuf::size() {
  return m_buf.size();
}

uint8_t *DwarfBuf::getBuf() {
  return &m_buf[0];
}

void DwarfBuf::print() {
  unsigned int i;
  for (i = 0; i < m_buf.size(); i++)
    printf("%x ", m_buf[i]);
  printf("\n");
}

void DwarfBuf::dwarf_op_const4s(int x) {
  byte(DW_OP_const4s);
  dword(x);
}

void DwarfBuf::dwarf_op_deref_size(uint8_t size) {
  byte(DW_OP_deref_size);
  byte(size);
}

void DwarfBuf::dwarf_sfp_expr(int offset, int scale) {
  byte(DW_OP_dup);
  dwarf_op_const4s(offset);
  byte(DW_OP_plus);
  dwarf_op_deref_size(sizeof(uint32_t));
  dwarf_op_const4s(scale);
  byte(DW_OP_mul);
  byte(DW_OP_plus);
}

void DwarfBuf::dwarf_cfa_sfp(uint8_t reg, int offset, int scale) {
  DwarfBuf b;
  byte(DW_CFA_val_expression);
  byte(reg);
  b.dwarf_sfp_expr(offset, scale);
  /* this assumes expression fits in 127 bytes, else we have
   to LEB128 encode size */
  byte(b.size());
  dwarf_sfp_expr(offset, scale);
}

void DwarfBuf::dwarf_cfa_unwind_rsp() {
  byte(DW_CFA_val_expression);
  byte(RSP);
  /* instruction sequence of length 2 */
  byte(2);
  /* add 8 to RSP (reg 7) */
  byte(DW_OP_breg7);
  byte(8);
}

void DwarfBuf::dwarf_cfa_same_value(uint8_t reg) {
  byte(DW_CFA_same_value);
  byte(reg);
}

void DwarfBuf::dwarf_cfa_set_loc(uint64_t addr) {
  byte(DW_CFA_set_loc);
  qword(addr);
}

void DwarfBuf::dwarf_cfa_def_cfa(uint8_t reg, uint8_t offset) {
  byte(DW_CFA_def_cfa);
  byte(reg);
  byte(offset);
}

void DwarfBuf::dwarf_cfa_offset(uint8_t reg, uint8_t offset) {
  byte(DW_CFA_offset | reg);
  byte(offset);
}

void DwarfBuf::dwarf_cfa_offset_extended_sf(uint8_t reg, int8_t offset) {
  byte(DW_CFA_offset_extended_sf);
  byte(reg);
  byte(offset & 0x7f);
}

const char *DwarfInfo::lookupFile(const Unit *unit) {
  const char *file = nullptr;
  if (unit && unit->filepath()) {
    file = unit->filepath()->data();
  }
  if (file == nullptr || strlen(file) == 0) {
    return "anonFile";
  }
  return file;
}

void DwarfInfo::addLineEntries(TCRange range,
                               const Unit *unit,
                               const Op* instr,
                               FunctionInfo* f) {
  if (unit == nullptr || instr == nullptr) {
    // For stubs, just add line 0
    f->m_lineTable.push_back(LineEntry(range, 0));
    return;
  }
  Offset offset = unit->offsetOf(reinterpret_cast<PC>(instr));

  int lineNum = unit->getLineNumber(offset);
  if (lineNum >= 0) {
    f->m_lineTable.push_back(LineEntry(range, lineNum));
  }
}

void DwarfInfo::transferFuncs(DwarfChunk* from, DwarfChunk* to) {
  unsigned int size = from->m_functions.size();
  for (unsigned int i = 0; i < size; i++) {
    FunctionInfo* f = from->m_functions[i];
    f->m_chunk = to;
    to->m_functions.push_back(f);
  }
}

void DwarfInfo::compactChunks() {
  unsigned int i, j;
  for (i = 1; i < m_dwarfChunks.size(); i++) {
    if (m_dwarfChunks[i] == nullptr) {
      break;
    }
  }
  if (i >= m_dwarfChunks.size()) {
    m_dwarfChunks.push_back(nullptr);
  }
  DwarfChunk* chunk = new DwarfChunk();
  for (j = 0; j < i; j++) {
    transferFuncs(m_dwarfChunks[j], chunk);
    // unregister chunk from gdb and free chunk
    unregister_gdb_chunk(m_dwarfChunks[j]);
    delete(m_dwarfChunks[j]);
    m_dwarfChunks[j] = nullptr;
  }
  m_dwarfChunks[i] = chunk;
#if !defined(__APPLE__) && !defined(__FreeBSD__)
  // register compacted chunk with gdb
  ElfWriter e = ElfWriter(chunk);
#endif
}

static Mutex s_lock(RankLeaf);

DwarfChunk* DwarfInfo::addTracelet(TCRange range, const char* name,
  const Func *func, const Op* instr, bool exit, bool inPrologue) {
  DwarfChunk* chunk = nullptr;
  FunctionInfo* f = new FunctionInfo(range, exit);
  const Unit* unit = func ? func->unit(): nullptr;
  if (name) {
    f->name = std::string(name);
  } else {
    assert(func != nullptr);
    f->name = lookupFunction(func, exit, inPrologue, true);
    const StringData* const *names = func->localNames();
    for (int i = 0; i < func->numNamedLocals(); i++) {
      f->m_namedLocals.push_back(names[i]->toCppString());
    }
  }
  f->file = lookupFile(unit);

  TCA start = range.begin();
  const TCA end = range.end();

  Lock lock(s_lock);
  FuncDB::iterator it = m_functions.lower_bound(range.begin());
  FunctionInfo* fi = it->second;
  if (it != m_functions.end() && fi->name == f->name &&
      fi->file == f->file &&
      start > fi->range.begin() &&
      end > fi->range.end()) {
    // XXX: verify that overlapping address come from jmp fixups
    start = fi->range.end();
    fi->range.extend(end);
    m_functions[end] = fi;
    m_functions.erase(it);
    delete(f);
    f = m_functions[end];
    assert(f->m_chunk != nullptr);
    f->m_chunk->clearSynced();
    f->clearPerfSynced();
  } else {
    m_functions[end] = f;
  }

  addLineEntries(TCRange(start, end, range.isAstubs()), unit, instr, f);

  if (f->m_chunk == nullptr) {
    if (m_dwarfChunks.size() == 0 || m_dwarfChunks[0] == nullptr) {
      // new chunk of base size
      chunk = new DwarfChunk();
      m_dwarfChunks.push_back(chunk);
    } else if (m_dwarfChunks[0]->m_functions.size()
                 < RuntimeOption::EvalGdbSyncChunks) {
      // reuse first chunk
      chunk = m_dwarfChunks[0];
      chunk->clearSynced();
    } else {
      // compact chunks
      compactChunks();
      m_dwarfChunks[0] = chunk = new DwarfChunk();
    }
    chunk->m_functions.push_back(f);
    f->m_chunk = chunk;
  }

#if !defined(__APPLE__) && !defined(__FreeBSD__)
  if (f->m_chunk->m_functions.size() >= RuntimeOption::EvalGdbSyncChunks) {
    ElfWriter e = ElfWriter(f->m_chunk);
  }
#endif

  return f->m_chunk;
}

void DwarfInfo::syncChunks() {
  unsigned int i;
  Lock lock(s_lock);
  for (i = 0; i < m_dwarfChunks.size(); i++) {
    if (m_dwarfChunks[i] && !m_dwarfChunks[i]->isSynced()) {
      unregister_gdb_chunk(m_dwarfChunks[i]);
#if !defined(__APPLE__) && !defined(__FreeBSD__)
      ElfWriter e = ElfWriter(m_dwarfChunks[i]);
#endif
    }
  }
}

}
}
