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
#include "hphp/runtime/vm/debug/elfwriter.h"
#include "hphp/runtime/vm/debug/gdb-jit.h"
#include <elf.h>
#include <gelf.h>
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "hphp/util/asm-x64.h"
#include "hphp/util/configs/jit.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"

using namespace HPHP::jit;

namespace HPHP {
namespace Debug {

TRACE_SET_MOD(debuginfo);
static const uint8_t CFA_OFFSET = 16;

void ElfWriter::logError(const std::string& msg) {
  perror("");
  std::cerr << msg << '\n';
}

int ElfWriter::dwarfCallback(LIBDWARF_CALLBACK_NAME_TYPE name, int size,
                             Dwarf_Unsigned type, Dwarf_Unsigned flags,
                             Dwarf_Unsigned /*link*/, Dwarf_Unsigned /*info*/) {
  if (!strncmp(name, ".rel", 4))
    return 0;
  return newSection(name, size, type, flags);
}

bool ElfWriter::initElfHeader() {
  if (elf_version(EV_CURRENT) == EV_NONE) {
    logError("ELF library initialization failed");
    return false;
  }

  m_elf = elf_begin(m_fd, ELF_C_WRITE, 0);
  if (!m_elf) {
    logError("Unable to create elf with elf_begin()");
    return false;
  }

  m_ehdr = elf64_newehdr(m_elf);
  if (!m_ehdr) {
    logError("Unable to create elf header with elf64_newehdr()");
    return false;
  }

  m_ehdr->e_ident[EI_MAG0] = ELFMAG0;
  m_ehdr->e_ident[EI_MAG1] = ELFMAG1;
  m_ehdr->e_ident[EI_MAG2] = ELFMAG2;
  m_ehdr->e_ident[EI_MAG3] = ELFMAG3;
  m_ehdr->e_ident[EI_CLASS] = ELFCLASS64;
  m_ehdr->e_ident[EI_DATA] = ELFDATA2LSB;
  m_ehdr->e_ident[EI_VERSION] = EV_CURRENT;
  m_ehdr->e_machine = EM_X86_64;
  m_ehdr->e_type = ET_EXEC;
  m_ehdr->e_version = EV_CURRENT;

  return true;
}

int ElfWriter::addSectionString(const std::string& name) {
  int off = m_strtab.size();
  for (unsigned int i = 0; i < name.size(); i++) {
    m_strtab.push_back(name[i]);
  }
  m_strtab.push_back('\0');
  return off;
}

void ElfWriter::initStrtab() {
  addSectionString("");
}

#if defined(LIBDWARF_USE_INIT_C)
bool ElfWriter::initDwarfProducer() {
  Dwarf_Error error = 0;
  /* m_dwarfProducer is the handle used for interaction for libdwarf */
  m_dwarfProducer =
    dwarf_producer_init_c(
    DW_DLC_WRITE | DW_DLC_SIZE_64 | DW_DLC_SYMBOLIC_RELOCATIONS,
    g_dwarfCallback,
    nullptr,
    nullptr,
    reinterpret_cast<Dwarf_Ptr>(this),
    &error);
  if (m_dwarfProducer == reinterpret_cast<Dwarf_P_Debug>(DW_DLV_BADADDR)) {
    logError("Unable to create dwarf producer");
    return false;
  }
  return true;
}
#else
bool ElfWriter::initDwarfProducer() {
  Dwarf_Error error = 0;
  auto ret = dwarf_producer_init(
    DW_DLC_WRITE | DW_DLC_SIZE_64 | DW_DLC_SYMBOLIC_RELOCATIONS,
    g_dwarfCallback,
    nullptr,
    nullptr,
    reinterpret_cast<Dwarf_Ptr>(this),
    "x86_64",
    "V2",
    nullptr,
    &m_dwarfProducer,
    &error);
  if (ret != DW_DLV_OK) {
    logError("Unable to create dwarf producer");
    return false;
  }
  return true;
}
#endif

Dwarf_P_Die ElfWriter::addFunctionInfo(FunctionInfo* f, Dwarf_P_Die type) {
  Dwarf_Error error = 0;

  /* top level DIE for each function */
  Dwarf_P_Die func = dwarf_new_die(m_dwarfProducer,
    DW_TAG_subprogram, nullptr, nullptr, nullptr, nullptr, &error);
  if (reinterpret_cast<Dwarf_Addr>(func) == DW_DLV_BADADDR) {
    logError("unable to create child DIE");
    return nullptr;
  }

  Dwarf_Signed file;
  FileDB::iterator it = m_fileDB.find(f->file);
  /* if this function is from an unseen file, register file name
   * and get index to file name */
  if (it == m_fileDB.end()) {
    file = dwarf_add_file_decl(m_dwarfProducer,
      (char *)f->file, 0, 0, 1000, &error);
    if (file == DW_DLV_NOCOUNT) {
      logError("unable to add file declaration");
      return nullptr;
    }
    m_fileDB[f->file] = file;
  } else {
    file = it->second;
  }

  /* add function name attribute to function DIE */
  Dwarf_P_Attribute at;
  at = dwarf_add_AT_name(func, (char *)f->name.c_str(), &error);
  if (reinterpret_cast<Dwarf_Addr>(at) == DW_DLV_BADADDR) {
    logError("unable to add name attribute to function");
    return nullptr;
  }

  /* Add lower PC bound to function DIE */
  at = dwarf_add_AT_targ_address(m_dwarfProducer, func, DW_AT_low_pc,
    reinterpret_cast<Dwarf_Unsigned>(f->range.begin()), 0, &error);
  if (reinterpret_cast<Dwarf_Addr>(at) == DW_DLV_BADADDR) {
    logError("unable to add low_pc attribute to function");
    return nullptr;
  }

  /* add upper PC bound to function DIE */
  at = dwarf_add_AT_targ_address(m_dwarfProducer, func, DW_AT_high_pc,
    reinterpret_cast<Dwarf_Unsigned>(f->range.end()), 0, &error);
  if (reinterpret_cast<Dwarf_Addr>(at) == DW_DLV_BADADDR) {
    logError("unable to add high_pc attribute to function");
    return nullptr;
  }

  /* register line number information for function:
   * 1. register start address */
  Dwarf_Unsigned u;
  u = dwarf_lne_set_address(m_dwarfProducer,
    reinterpret_cast<Dwarf_Addr>(f->range.begin()), 0, &error);
  if (u != 0) {
    logError("unable to set line start address");
    return nullptr;
  }

  /* 2. register line number info for each tracelet in function */
  std::vector<LineEntry>::iterator it2;
  for (it2 = f->m_lineTable.begin(); it2 != f->m_lineTable.end(); it2++) {
    u = dwarf_add_line_entry(m_dwarfProducer,
      file, reinterpret_cast<Dwarf_Addr>(it2->range.begin()), it2->lineNumber,
      0, 1, 0, &error);
    if (u != 0) {
      logError("unable to add line entry");
      return nullptr;
    }
    TRACE(1, "elfwriter tracelet: %s %p %p\n",
          m_filename.c_str(), it2->range.begin(), it2->range.end());
  }

  /* 3. register end address of function */
  u = dwarf_lne_end_sequence(m_dwarfProducer,
    reinterpret_cast<Dwarf_Addr>(f->range.end()), &error);
  if (u != 0) {
    logError("unable to set line end address");
    return nullptr;
  }

  {
    /* 4. register frame base of function */
    Dwarf_P_Expr locExpr = dwarf_new_expr(m_dwarfProducer, &error);
    if (locExpr == nullptr) {
      logError("unable to create new location expression");
      return nullptr;
    }

    u = dwarf_add_expr_gen(locExpr, DW_OP_call_frame_cfa, 0, 0, &error);
    if (u == DW_DLV_NOCOUNT) {
      logError("unable to add subexpression to location expression");
      return nullptr;
    }

    u = dwarf_add_expr_gen(locExpr, DW_OP_const1u, CFA_OFFSET, 0, &error);
    if (u == DW_DLV_NOCOUNT) {
      logError("unable to add subexpression to location expression");
      return nullptr;
    }

    u = dwarf_add_expr_gen(locExpr, DW_OP_minus, 0, 0, &error);
    if (u == DW_DLV_NOCOUNT) {
      logError("unable to add subexpression to location expression");
      return nullptr;
    }

    Dwarf_P_Attribute frameBaseAttr = dwarf_add_AT_location_expr(
      m_dwarfProducer, func, DW_AT_frame_base, locExpr, &error);
    if (reinterpret_cast<Dwarf_Addr>(frameBaseAttr) == DW_DLV_BADADDR) {
      logError("unable to add frame_base attribute");
      return nullptr;
    }
  }

  /* 5. register all the named locals of function */
  Dwarf_P_Die lastLocal = nullptr;
  int i = 1;
  for (std::vector<std::string>::iterator it = f->m_namedLocals.begin();
       it != f->m_namedLocals.end(); it++) {
    Dwarf_P_Die localVar = dwarf_new_die(m_dwarfProducer,
      DW_TAG_variable, nullptr, nullptr, nullptr, nullptr, &error);
    if (reinterpret_cast<Dwarf_Addr>(localVar) == DW_DLV_BADADDR) {
      logError("unable to create new DIE for local variable");
      return nullptr;
    }

    /* Create location expression defined w.r.t DW_AT_frame_base */
    Dwarf_P_Expr locExpr = dwarf_new_expr(m_dwarfProducer, &error);
    if (locExpr == nullptr) {
      logError("unable to create new location expression");
      return nullptr;
    }

    u = dwarf_add_expr_gen(locExpr, DW_OP_fbreg,
      -(i * sizeof(TypedValue)), 0, &error);
    ++i;
    if (u == DW_DLV_NOCOUNT) {
      logError("unable to add subexpression to location expression");
      return nullptr;
    }

    Dwarf_P_Attribute locAttr = dwarf_add_AT_location_expr(m_dwarfProducer,
      localVar, DW_AT_location, locExpr, &error);
    if (reinterpret_cast<Dwarf_Addr>(locAttr) == DW_DLV_BADADDR) {
      logError("unable to add location attribute to local variable");
      return nullptr;
    }

    Dwarf_P_Attribute nameAttr = dwarf_add_AT_name(localVar, const_cast<char *>(it->data()), &error);
    if (reinterpret_cast<Dwarf_Addr>(nameAttr) == DW_DLV_BADADDR) {
      logError("unable to add name attribute to local variable");
      return nullptr;
    }

    Dwarf_P_Attribute varTypeAttr = dwarf_add_AT_reference(m_dwarfProducer,
      localVar, DW_AT_type, type, &error);
    if (reinterpret_cast<Dwarf_Addr>(varTypeAttr) == DW_DLV_BADADDR) {
      logError("unable to add type attribute to local variable DIE");
      return nullptr;
    }

    Dwarf_P_Die res = 0;
    if (lastLocal != nullptr) {
      res = dwarf_die_link(localVar, nullptr, nullptr, lastLocal, nullptr, &error);
    } else {
      res = dwarf_die_link(localVar, func, nullptr, nullptr, nullptr, &error);
    }
    if (reinterpret_cast<Dwarf_Addr>(res) == DW_DLV_BADADDR) {
      logError("unable to link die");
      return nullptr;
    }
    lastLocal = localVar;
  }

  return func;
}

Dwarf_P_Die ElfWriter::makeLocalTypeDie() {
  Dwarf_Error error = 0;
  Dwarf_P_Die typedValueType = dwarf_new_die(m_dwarfProducer,
    DW_TAG_structure_type, nullptr, nullptr, nullptr, nullptr, &error);
  if (reinterpret_cast<Dwarf_Addr>(typedValueType) == DW_DLV_BADADDR) {
    logError("unable to create new DIE for TypedValue type");
    return nullptr;
  }

  /* hard coding the name of 'HPHP::TypedValue' */
  Dwarf_P_Attribute at;
  at = dwarf_add_AT_name(typedValueType, "HPHP::TypedValue", &error);
  if (reinterpret_cast<Dwarf_Addr>(at) == DW_DLV_BADADDR) {
    logError("unable to add name attribute to TypedValue type DIE");
    return nullptr;
  }

  at = dwarf_add_AT_flag(m_dwarfProducer,
    typedValueType, DW_AT_declaration, 1, &error);
  if (reinterpret_cast<Dwarf_Addr>(at) == DW_DLV_BADADDR) {
    logError("unable to add declaration attribute to TypedValue type DIE");
    return nullptr;
  }

  at = dwarf_add_AT_unsigned_const(m_dwarfProducer,
    typedValueType, DW_AT_byte_size, sizeof(TypedValue), &error);
  if (reinterpret_cast<Dwarf_Addr>(at) == DW_DLV_BADADDR) {
    logError("unable to add byte_size attribute to TypedValue type DIE");
    return nullptr;
  }
  return typedValueType;
}

bool ElfWriter::addSymbolInfo(DwarfChunk* d) {
  Dwarf_Error error = 0;

  /* create a top level DIE (debug information entry)
   * all subsequent DIEs' will be children of this DIE
   */
  Dwarf_P_Die codeUnit = dwarf_new_die(m_dwarfProducer,
    DW_TAG_compile_unit, nullptr, nullptr, nullptr, nullptr, &error);
  if (reinterpret_cast<Dwarf_Addr>(codeUnit) == DW_DLV_BADADDR) {
    logError("unable to create code unit DIE");
    return false;
  }

  Dwarf_P_Die lastChild = nullptr;
  FuncPtrDB::iterator it;
  Dwarf_P_Die type = makeLocalTypeDie();
  if (type == nullptr) {
    logError("unable to create type DIE");
    return false;
  }
  Dwarf_P_Die linkRes;
  linkRes = dwarf_die_link(type, codeUnit, nullptr, nullptr, nullptr, &error);
  if (reinterpret_cast<Dwarf_Addr>(linkRes) == DW_DLV_BADADDR) {
    logError("unable to link die");
    return false;
  }

  for (it = d->m_functions.begin(); it != d->m_functions.end(); it++) {
    /* for each function, add DIE entries with information about name,
     * line number, file, etc */
    Dwarf_P_Die func = addFunctionInfo(*it, type);
    if (func == nullptr) {
      logError("unable to create child DIE");
      return false;
    }

    if (lastChild) {
      linkRes = dwarf_die_link(func, nullptr, nullptr, lastChild, nullptr, &error);
    } else {
      linkRes = dwarf_die_link(func, codeUnit, nullptr, nullptr, nullptr, &error);
    }
    if (reinterpret_cast<Dwarf_Addr>(linkRes) == DW_DLV_BADADDR) {
      logError("unable to link die");
      return false;
    }
    lastChild = func;
  }

  /* register top level DIE */
  Dwarf_Unsigned res = dwarf_add_die_to_debug(m_dwarfProducer,
    codeUnit, &error);
  if (res != DW_DLV_OK) {
    logError("unable to add DIE to DWARF");
    return false;
  }

  return true;
}

bool ElfWriter::addFrameInfo(DwarfChunk* d) {
  Dwarf_Error error = 0;
  DwarfBuf& b = d->m_buf;
  b.clear();
  /* Define common set of rules for unwinding frames in the VM stack*/

  /* Frame pointer (CFA) for previous frame is in RBP + 16 */
  b.dwarf_cfa_def_cfa(RBP, CFA_OFFSET);
  /* Previous RIP is at CFA - 1 . DWARF_DATA_ALIGN (8) */
  b.dwarf_cfa_offset_extended_sf(RIP, -1);
  /* Previous RBP is at CFA - 2 . DWARF_DATA_ALIGN (8) */
  b.dwarf_cfa_offset_extended_sf(RBP, -2);
  /*
   * RSP is unchanged in VM frames, except for some rare cases with
   * calls to functions that we assume don't throw.  (Technically
   * debug information will be wrong if we stop under one of those
   * cases.)
   *
   * Note: if rVmSp is ever changed to refer to rsp, this code needs
   * to change.
   */
  b.dwarf_cfa_same_value(RSP);

  /* register above rules in a CIE (common information entry) */
  Dwarf_Signed cie_index = dwarf_add_frame_cie(
    m_dwarfProducer,
    "",
    DWARF_CODE_ALIGN,
    DWARF_DATA_ALIGN,
    RIP,
    (void *)b.getBuf(),
    b.size(),
    &error
  );
  if (cie_index == DW_DLV_NOCOUNT) {
    logError("Unable to add CIE frame");
    return false;
  }

  /* for each tracelet, register tracelet address ranges in
   * an FDE (Frame Description entry) */
  FuncPtrDB::iterator it;
  for (it = d->m_functions.begin(); it != d->m_functions.end(); it++) {
    Dwarf_P_Fde fde = dwarf_new_fde(m_dwarfProducer, &error);
    if (reinterpret_cast<Dwarf_Addr>(fde) == DW_DLV_BADADDR) {
      logError("Unable to create FDE");
      return false;
    }
    DwarfBuf buf;
    int err = dwarf_insert_fde_inst_bytes(
      m_dwarfProducer, fde, buf.size(), buf.getBuf(), &error);
    if (err == DW_DLV_ERROR) {
      logError("Unable to add instructions to fde");
      return false;
    }
    Dwarf_Unsigned fde_index = dwarf_add_frame_fde(
      m_dwarfProducer, fde, 0, cie_index,
      (Dwarf_Unsigned)((*it)->range.begin()),
      (*it)->range.size(),
      0, &error);
    if (fde_index == DW_DLV_BADADDR) {
      logError("Unable to add FDE");
      return false;
    }
  }
  return true;
}

int ElfWriter::newSection(LIBDWARF_CALLBACK_NAME_TYPE name,
  uint64_t size, uint32_t type, uint64_t flags,
  uint64_t addr/* = 0*/) {
  Elf_Scn *scn = elf_newscn(m_elf);
  if (!scn) {
    logError("Unable to create new section");
    return -1;
  }
  Elf64_Shdr *sectionHdr = elf64_getshdr(scn);
  if (!sectionHdr) {
    logError("Unable to create section header");
    return -1;
  }
  int nameOffset = addSectionString(name);
  sectionHdr->sh_name = nameOffset;
  sectionHdr->sh_type = type;
  sectionHdr->sh_flags = flags;
  sectionHdr->sh_size = size;
  sectionHdr->sh_addr = addr;
  sectionHdr->sh_offset = 0;
  sectionHdr->sh_link = 0;
  sectionHdr->sh_info = 0;
  sectionHdr->sh_addralign = 1;
  sectionHdr->sh_entsize = 0;

  return elf_ndxscn(scn);
}

bool ElfWriter::addSectionData(int section_index, void *data, uint64_t size) {
  Elf_Scn *scn = elf_getscn(m_elf, section_index);
  if (!scn) {
    logError("Unable to retrieve section number");
    return false;
  }
  Elf_Data *elfData = elf_newdata(scn);
  if (!elfData) {
    logError("Unable to add section data");
    return false;
  }
  elfData->d_buf = data;
  elfData->d_type = ELF_T_BYTE;
  elfData->d_size = size;
  elfData->d_off = 0;
  elfData->d_align = 1;
  elfData->d_version = EV_CURRENT;
  return true;
}

bool ElfWriter::writeDwarfInfo() {
  Dwarf_Signed sections = dwarf_transform_to_disk_form (m_dwarfProducer, 0);

  Dwarf_Signed i = 0;
  Dwarf_Signed elf_section_index = 0;
  Dwarf_Unsigned length = 0;

  for (i = 0; i < sections; i++) {
    Dwarf_Ptr bytes = dwarf_get_section_bytes(
      m_dwarfProducer, 0, &elf_section_index, &length, 0);

    if (!addSectionData(elf_section_index, bytes, length)) {
      logError("Unable to create section");
      return false;
    }
  }
  return true;
}

int ElfWriter::writeStringSection() {
  int section = -1;
  if ((section = newSection(
      ".shstrtab", m_strtab.size(), SHT_STRTAB, SHF_STRINGS)) < 0) {
    logError("unable to create string section");
    return -1;
  }
  if (!addSectionData(section, &m_strtab[0], m_strtab.size())) {
    logError("unable to add string data");
    return -1;
  }
  return section;
}

int ElfWriter::writeTextSection() {
  int section = -1;
  auto const& code = tc::code();
  if ((section = newSection(
         ".text.tracelets", code.codeSize(),
         SHT_NOBITS, SHF_ALLOC | SHF_EXECINSTR,
         reinterpret_cast<uint64_t>(code.base()))) < 0) {
    logError("unable to create text section");
    return -1;
  }
  if (!addSectionData(section, nullptr, code.codeSize())) {
    logError("unable to add text data");
    return -1;
  }
  return section;
}

ElfWriter::ElfWriter(DwarfChunk* d):
  m_fd(-1), m_elf(nullptr), m_dwarfProducer(nullptr) {
  off_t elf_size;
  char *symfile;

  m_filename = std::string("/tmp/vm_dwarf.XXXXXX");
  m_fd = mkstemp((char *)m_filename.c_str());
  if (m_fd < 0) {
    logError("Unable to open file for writing.");
    return;
  }
  if (!initElfHeader())
    return;
  initStrtab();
  if (!initDwarfProducer())
    return;
  if (!addFrameInfo(d))
    return;
  if (!addSymbolInfo(d))
    return;
  if (!writeDwarfInfo())
    return;
  if (!writeTextSection())
    return;
  int stringIndex;
  if ((stringIndex = writeStringSection()) < 0) {
    logError("Unable to create string section");
    return;
  }
  m_ehdr->e_shstrndx = stringIndex;

  if ((elf_size = elf_update(m_elf, ELF_C_WRITE)) == -1) {
    logError("Error writing ELF to disk");
    return;
  }

  if (lseek(m_fd, 0, SEEK_SET) != 0) {
    logError("Unable to seek to beginning of ELF file");
    return;
  }

  symfile = (char*)malloc(elf_size);
  if (read(m_fd, (void *)symfile, elf_size) != elf_size) {
    free(symfile);
    logError("Unable to read elf file");
    return;
  }
  register_gdb_hook(symfile, elf_size, d);
  d->setSynced();
}

ElfWriter::~ElfWriter() {
  if (m_elf != nullptr)
    elf_end(m_elf);
  if (m_fd != -1)
    close(m_fd);
  if (!Cfg::Jit::KeepDbgFiles) {
    unlink(m_filename.c_str());
  }
  if (m_dwarfProducer != nullptr)
    dwarf_producer_finish(m_dwarfProducer, 0);
}

}
}
