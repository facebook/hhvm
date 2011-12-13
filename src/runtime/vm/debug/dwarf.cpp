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
#include <stdio.h>
#include "dwarf.h"
#include "debug.h"
#include "elfwriter.h"
#include "gdb-jit.h"

#include <runtime/base/types.h>
#include <runtime/base/execution_context.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/translator/translator-inline.h>

using namespace std;
using namespace HPHP::VM::Transl;

namespace HPHP {
namespace VM {
namespace Debug {


int g_dwarfCallback(char *name, int size, Dwarf_Unsigned type,
            Dwarf_Unsigned flags, Dwarf_Unsigned link, Dwarf_Unsigned info,
            Dwarf_Unsigned *sect_name_index, Dwarf_Ptr handle, int *error) {
  ElfWriter *e = reinterpret_cast<ElfWriter *>(handle);
  return e->dwarfCallback(name, size, type, flags, link, info);
}

void DwarfBuf::byte(uint8_t c) {
  m_buf.push_back(c);
}

void DwarfBuf::byte(int off, uint8_t c) {
  m_buf[off] = c;
}

void DwarfBuf::word(uint16_t w) {
  byte(w & 0xff);
  byte((w >> 8) & 0xff);
}

void DwarfBuf::word(int off, uint16_t w) {
  byte(off, (w & 0xff));
  byte(off, ((w >> 8) & 0xff));
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

DwarfBuf::DwarfBuf() {
}

DwarfInfo::DwarfInfo() {
}

std::string DwarfInfo::lookupFunction(const Unit *unit,
                                      const Opcode *instr,
                                      bool exit/* = false*/,
                                      bool inPrologue/* = false*/,
                                      bool pseudoWithFileName/* = false*/,
                                      bool withPHPPrefix/* = false*/) {
  // TODO: mangle the namespace and name?
  string fname(withPHPPrefix ? "PHP::" : "");
  if (unit == NULL || instr == NULL) {
    fname += "#anonFunc";
    return fname;
  }
  Func *f = unit->getFunc(unit->offsetOf(instr));
  if (f != NULL) {
    if (!strcmp(f->name(), "")) {
      if (pseudoWithFileName) {
        fname += f->m_unit->m_filepath->data();
        fname += '$';
      }
      if (!exit) {
        fname += "__pseudoMain";
      } else {
        fname += "__exit";
      }
      return fname;
    }
    fname += f->name();
    if (inPrologue)
      fname += "$prologue";
    return fname;
  }
  fname += "#anonFunc";
  return fname;
}

const char *DwarfInfo::lookupFile(const Unit *unit) {
  const char *file = NULL;
  if (unit && unit->m_filepath) {
    file = unit->m_filepath->data();
  }
  if (file == NULL || strlen(file) == 0) {
    return "anonFile";
  }
  return file;
}

void DwarfInfo::addLineEntries(TCA start, TCA end, const Unit *unit,
  const Opcode *instr, FunctionInfo* f) {
  if (unit == NULL || instr == NULL) {
    // For stubs, just add line 0
    f->m_lineTable.push_back(LineEntry(start, end, 0));
    return;
  }
  Offset offset = unit->offsetOf(instr);

  int lineNum = unit->getLineNumber(offset);
  if (lineNum >= 0) {
    f->m_lineTable.push_back(LineEntry(start, end, lineNum));
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
    if (m_dwarfChunks[i] == NULL) {
      break;
    }
  }
  if (i >= m_dwarfChunks.size()) {
    m_dwarfChunks.push_back(NULL);
  }
  DwarfChunk* chunk = new DwarfChunk();
  for (j = 0; j < i; j++) {
    transferFuncs(m_dwarfChunks[j], chunk);
    // unregister chunk from gdb and free chunk
    unregister_gdb_chunk(m_dwarfChunks[j]);
    delete(m_dwarfChunks[j]);
    m_dwarfChunks[j] = NULL;
  }
  m_dwarfChunks[i] = chunk;
  // register compacted chunk with gdb
  ElfWriter e = ElfWriter(chunk);
}

DwarfChunk* DwarfInfo::addTracelet(TCA start, TCA end, const Unit *unit,
  const Opcode *instr, bool exit, bool inPrologue) {
  DwarfChunk* chunk = NULL;
  FunctionInfo* f = new FunctionInfo(start, end, exit);
  f->name = lookupFunction(unit, instr, exit, inPrologue, true, true);
  f->file = lookupFile(unit);

  FuncDB::iterator it = m_functions.lower_bound(start);
  if (it != m_functions.end() && it->second->name == f->name
    && it->second->file == f->file
    && start > it->second->start && end > it->second->end) {
    // XXX: verify that overlapping address come from jmp fixups
    start = it->second->end;
    it->second->end = end;
    m_functions[end] = it->second;
    m_functions.erase(it);
    delete(f);
    f = m_functions[end];
  } else {
    m_functions[end] = f;
  }
  addLineEntries(start, end, unit, instr, f);

  if (f->m_chunk == NULL) {
    if (m_dwarfChunks.size() == 0 || m_dwarfChunks[0] == NULL) {
      // new chunk of base size
      chunk = new DwarfChunk();
      m_dwarfChunks.push_back(chunk);
    } else if (m_dwarfChunks[0]->m_functions.size() < BASE_FUNCS_PER_CHUNK) {
      // reuse first chunk
      chunk = m_dwarfChunks[0];
    } else {
      // compact chunks
      compactChunks();
      m_dwarfChunks[0] = chunk = new DwarfChunk();
    }
    chunk->m_functions.push_back(f);
    f->m_chunk = chunk;
  }

  return f->m_chunk;
}

}
}
}
