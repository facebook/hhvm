/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/util/eh-frame.h"

#include <folly/Optional.h>

#include <string>
#include <vector>

namespace HPHP { namespace Debug {

using jit::TCA;

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

#if (defined(FACEBOOK) || defined(LIBDWARF_CONST_NAME))
#define LIBDWARF_CALLBACK_NAME_TYPE const char*
#else
#define LIBDWARF_CALLBACK_NAME_TYPE char*
#endif

extern int g_dwarfCallback(
  LIBDWARF_CALLBACK_NAME_TYPE name, int size, Dwarf_Unsigned type,
  Dwarf_Unsigned flags, Dwarf_Unsigned link, Dwarf_Unsigned info,
  Dwarf_Unsigned *sect_name_index, Dwarf_Ptr handle, int *error);

struct TCRange {
  TCRange() : m_start(nullptr), m_end(nullptr), m_isAcold(false) {
    assertx(!isValid());
  }
  TCRange(const TCA start, const TCA end, bool isAcold) :
    m_start(start), m_end(end), m_isAcold(isAcold) { V(); }

  TCRange& operator=(const TCRange& r) {
    m_start = r.m_start;
    m_end = r.m_end;
    m_isAcold = r.m_isAcold;
    V();
    return *this;
  }

  bool isValid() const {
    assertx(bool(m_start) == bool(m_end));
    assertx(!m_start || m_start < m_end);
    assertx(!m_start || (m_end - m_start) < (1ll << 32));
    return bool(m_start);
  }
  bool isAcold() const { return m_isAcold; }
  TCA begin() const { V(); return m_start; }
  TCA end() const   { V(); return m_end; };
  uint32_t size() const   { V(); return m_end - m_start; }

  void extend(const TCA newEnd) {
    assertx(newEnd >= m_end);
    m_end = newEnd;
    V();
  }

private:
  void V() const { assertx(isValid()); }

private:
  TCA m_start, m_end;
  bool m_isAcold;
};

struct DwarfBuf {
  void byte(uint8_t c);

  void clear();
  int size();
  uint8_t *getBuf();
  void print();

  void dwarf_cfa_def_cfa(uint8_t reg, uint8_t offset);
  void dwarf_cfa_same_value(uint8_t reg);
  void dwarf_cfa_offset_extended_sf(uint8_t reg, int8_t offset);

private:
  std::vector<uint8_t> m_buf;
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
  std::vector<FunctionInfo *> m_functions;
  char *m_symfile;
  bool m_synced;
  DwarfChunk() : m_symfile(nullptr), m_synced(false) {}
  void setSynced() { m_synced = true; }
  void clearSynced() { m_synced = false; }
  bool isSynced() const { return m_synced; }
};

typedef std::map<TCA, FunctionInfo* > FuncDB;
typedef std::vector<FunctionInfo* > FuncPtrDB;

struct DwarfInfo {
  typedef std::map<TCA, jit::TransRec> TransDB;

  std::vector<DwarfChunk*> m_dwarfChunks;
  /* Array of chunks indexed by lg(#functions in chunk) + 1.
   * i.e. m_dwarfChunk[i] = pointer to chunk with
   * 2^(i-1) * RuntimeOption::EvalGdbSyncChunks functions, or NULL if
   * there is no such chunk. The first chunk m_dwarfChunks[0] is special in
   * that it can be partially full. All other chunks are completely full.
   */
  FuncDB m_functions;

  const char *lookupFile(const Unit *unit);
  void addLineEntries(TCRange range, const Unit *unit,
                      PC instr, FunctionInfo* f);
  void transferFuncs(DwarfChunk* from, DwarfChunk* to);
  void compactChunks();
  DwarfChunk* addTracelet(TCRange range,
                          folly::Optional<std::string> name,
                          const Func* func,
                          PC instr,
                          bool exit,
                          bool inPrologue);
  void syncChunks();
};

}}

#endif
