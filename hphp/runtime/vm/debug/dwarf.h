/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef HPHP_DWARF_H_
#define HPHP_DWARF_H_

#include <string>

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/translator.h"
#include <libdwarf.h>
#include <dwarf.h>
#include <vector>

namespace HPHP {
namespace Debug {

using JIT::TCA;

typedef enum {
  RAX,
  RDX,
  RCX,
  RBX,
  RSI,
  RDI,
  RBP,
  RSP,
  R8,
  R9,
  R10,
  R11,
  R12,
  R13,
  R14,
  R15,
  RIP
} x86_64_regnum_t;

const int DWARF_CODE_ALIGN = 1;
const int DWARF_DATA_ALIGN = 8;

extern int g_dwarfCallback(char *name, int size, Dwarf_Unsigned type,
  Dwarf_Unsigned flags, Dwarf_Unsigned link, Dwarf_Unsigned info,
  Dwarf_Unsigned *sect_name_index, Dwarf_Ptr handle, int *error);

class TCRange {
  TCA m_start, m_end;
  bool m_isAstubs;
  void V() const { assert(isValid()); }
 public:
  TCRange() : m_start(nullptr), m_end(nullptr), m_isAstubs(false) {
    assert(!isValid());
  }
  TCRange(const TCA start, const TCA end, bool isAstubs) :
    m_start(start), m_end(end), m_isAstubs(isAstubs) { V(); }

  TCRange& operator=(const TCRange& r) {
    m_start = r.m_start;
    m_end = r.m_end;
    m_isAstubs = r.m_isAstubs;
    V();
    return *this;
  }

  bool isValid() const {
    assert(bool(m_start) == bool(m_end));
    assert(!m_start || m_start < m_end);
    assert(!m_start || (m_end - m_start) < (1ll << 32));
    return bool(m_start);
  }
  bool isAstubs() const { return m_isAstubs; }
  TCA begin() const { V(); return m_start; }
  TCA end() const   { V(); return m_end; };
  uint32_t size() const   { V(); return m_end - m_start; }

  void extend(const TCA newEnd) {
    assert(newEnd >= m_end);
    m_end = newEnd;
    V();
  }
};

struct DwarfBuf {
  vector<uint8_t> m_buf;

  void byte(uint8_t c);
  void byte(int off, uint8_t c);
  void word(uint16_t w);
  void word(int off, uint16_t w);
  void dword(uint32_t d);
  void dword(int off, uint32_t d);
  void qword(uint64_t q);
  void clear();
  int size();
  uint8_t *getBuf();
  void print();
  void dwarf_op_const4s(int x);
  void dwarf_op_deref_size(uint8_t size);
  void dwarf_sfp_expr(int offset, int scale);
  void dwarf_cfa_sfp(uint8_t reg, int offset, int scale);
  void dwarf_cfa_unwind_rsp();
  void dwarf_cfa_set_loc(uint64_t addr);
  void dwarf_cfa_same_value(uint8_t reg);
  void dwarf_cfa_def_cfa(uint8_t reg, uint8_t offset);
  void dwarf_cfa_offset(uint8_t reg, uint8_t offset);
  void dwarf_cfa_offset_extended_sf(uint8_t reg, int8_t offset);
};

struct LineEntry {
  TCRange range;
  int lineNumber;
  LineEntry(TCRange r, int l) : range(r), lineNumber(l) {}
};

struct DwarfChunk;

struct FunctionInfo {
  std::string name;
  const char *file;
  TCRange range;
  bool exit;
  bool m_perfSynced;
  std::vector<LineEntry> m_lineTable;
  std::vector<std::string> m_namedLocals;
  DwarfChunk* m_chunk;
  FunctionInfo() : m_chunk(nullptr) { }
  FunctionInfo(TCRange r, bool ex)
    : range(r), exit(ex), m_perfSynced(false),
    m_namedLocals(std::vector<std::string>()), m_chunk(nullptr) {}
  void setPerfSynced() { m_perfSynced = true; }
  void clearPerfSynced() { m_perfSynced = false; }
  bool perfSynced() const { return m_perfSynced; }
};

struct DwarfChunk {
  DwarfBuf m_buf;
  vector<FunctionInfo *> m_functions;
  char *m_symfile;
  bool m_synced;
  DwarfChunk() : m_symfile(nullptr), m_synced(false) {}
  void setSynced() { m_synced = true; }
  void clearSynced() { m_synced = false; }
  bool isSynced() const { return m_synced; }
};

typedef std::map<TCA, FunctionInfo* > FuncDB;
typedef vector<FunctionInfo* > FuncPtrDB;

struct DwarfInfo {
  typedef std::map<TCA, JIT::TransRec> TransDB;

  vector<DwarfChunk*> m_dwarfChunks;
  /* Array of chunks indexed by lg(#functions in chunk) + 1.
   * i.e. m_dwarfChunk[i] = pointer to chunk with
   * 2^(i-1) * RuntimeOption::EvalGdbSyncChunks functions, or NULL if
   * there is no such chunk. The first chunk m_dwarfChunks[0] is special in
   * that it can be partially full. All other chunks are completely full.
   */
  FuncDB m_functions;

  const char *lookupFile(const Unit *unit);
  void addLineEntries(TCRange range, const Unit *unit,
                      const Opcode *instr, FunctionInfo* f);
  void transferFuncs(DwarfChunk* from, DwarfChunk* to);
  void compactChunks();
  DwarfChunk* addTracelet(TCRange range, const char* name,
                          const Func* func, const Opcode *instr,
                          bool exit, bool inPrologue);
  void syncChunks();
};

}
}

#endif
