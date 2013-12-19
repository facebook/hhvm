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
#ifndef HPHP_ELFWRITER_H_
#define HPHP_ELFWRITER_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/debug/dwarf.h"
#include <elf.h>
#include <gelf.h>
#include <vector>
#include <string>

namespace HPHP {
namespace Debug {

struct ElfWriter {
  int m_fd;
  Elf *m_elf;
  std::string m_filename;
  Elf64_Ehdr *m_ehdr;
  vector<unsigned char> m_strtab;
  Dwarf_P_Debug m_dwarfProducer;
  typedef std::map<const char *, Dwarf_Signed> FileDB;
  FileDB m_fileDB;

  explicit ElfWriter(DwarfChunk* d);
  ~ElfWriter();
  int dwarfCallback(char *name, int size, Dwarf_Unsigned type,
    Dwarf_Unsigned flags, Dwarf_Unsigned link, Dwarf_Unsigned info);
  void logError(const std::string& msg);
  int addSectionString(const std::string& name);
  void initStrtab();
  bool initElfHeader();
  bool initDwarfProducer();
  Dwarf_P_Die makeLocalTypeDie();
  Dwarf_P_Die addFunctionInfo(FunctionInfo* f, Dwarf_P_Die type);
  bool addSymbolInfo(DwarfChunk* d);
  bool addFrameInfo(DwarfChunk* d);
  bool writeDwarfInfo();
  int newSection(
    char *name, uint64_t size, uint32_t type,
    uint64_t flags, uint64_t addr = 0);
  bool addSectionData(int section, void *data, uint64_t size);
  int writeStringSection();
  int writeTextSection();
};

}
}

#endif
