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
#include <libdwarf.h>
#ifdef LIBDWARF_USE_NEW_PRODUCER_API
#include <libdwarfp.h>
#endif
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

TRACE_SET_MOD(debuginfo)
static const uint8_t CFA_OFFSET = 16;

#ifdef LIBDWARF_USE_NEW_PRODUCER_API
#define DW_DLV_NOCOUNT ((Dwarf_Signed) -1)
#endif

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
    DW_DLC_SIZE_64 | DW_DLC_SYMBOLIC_RELOCATIONS,
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

Dwarf_Unsigned dwarf_add_expr_gen_compat(
  Dwarf_P_Expr expr,
  Dwarf_Small opcode,
  Dwarf_Unsigned val1,
  Dwarf_Unsigned val2,
  Dwarf_Error* error
) {
  Dwarf_Unsigned len = 0;
  int res = dwarf_add_expr_gen_a(expr, opcode, val1, val2, &len, error);
  if (res != DW_DLV_OK) {
    return DW_DLV_NOCOUNT;
  }

  return len;
}

Dwarf_P_Die dwarf_new_die_compat(
  Dwarf_P_Debug producer,
  Dwarf_Tag tag,
  Dwarf_P_Die parent,
  Dwarf_P_Die child,
  Dwarf_P_Die left,
  Dwarf_P_Die right,
  Dwarf_Error* error
) {
  Dwarf_P_Die die = nullptr;
  int res = dwarf_new_die_a(producer, tag, parent, child, left, right, &die, error);
  if (res != DW_DLV_OK) {
    return nullptr;
  }

  return die;
}

Dwarf_P_Attribute dwarf_add_AT_name_compat(Dwarf_P_Die die, char* name, Dwarf_Error* error) {
  Dwarf_P_Attribute attribute = nullptr;
  int res = dwarf_add_AT_name_a(die, name, &attribute, error);
  if (res != DW_DLV_OK) {
    return nullptr;
  }

  return attribute;
}

Dwarf_P_Attribute dwarf_add_AT_targ_address_compat(
  Dwarf_P_Debug producer,
  Dwarf_P_Die ownerDie,
  Dwarf_Half attr,
  Dwarf_Unsigned value,
  Dwarf_Unsigned symbolIndex,
  Dwarf_Error* error
) {
  Dwarf_P_Attribute attribute = nullptr;
  int res = dwarf_add_AT_targ_address_c(producer, ownerDie, attr, value, symbolIndex, &attribute, error);
  if (res != DW_DLV_OK) {
    return nullptr;
  }

  return attribute;
}


Dwarf_P_Attribute dwarf_add_AT_location_expr_compat(
  Dwarf_P_Debug producer,
  Dwarf_P_Die ownerDie,
  Dwarf_Half attr,
  Dwarf_P_Expr locExpr,
  Dwarf_Error* error
) {
  Dwarf_P_Attribute attribute = nullptr;
  int res = dwarf_add_AT_location_expr_a(producer, ownerDie, attr, locExpr, &attribute, error);
  if (res != DW_DLV_OK) {
    return nullptr;
  }

  return attribute;
}

Dwarf_P_Attribute dwarf_add_AT_reference_compat(
  Dwarf_P_Debug dbg,
  Dwarf_P_Die ownerDie,
  Dwarf_Half attr,
  Dwarf_P_Die otherDie,
  Dwarf_Error* error
) {
  Dwarf_P_Attribute attribute = nullptr;
  int res = dwarf_add_AT_reference_c(dbg, ownerDie, attr, otherDie, &attribute, error);
  if (res != DW_DLV_OK) {
    return nullptr;
  }

  return attribute;
}

Dwarf_P_Die dwarf_die_link_compat(
  Dwarf_P_Die newDie,
  Dwarf_P_Die parent,
  Dwarf_P_Die child,
  Dwarf_P_Die left,
  Dwarf_P_Die right,
  Dwarf_Error* error
) {
  int res = dwarf_die_link_a(newDie, parent, child, left, right, error);
  if (res != DW_DLV_OK) {
    return nullptr;
  }

  return newDie;
}

Dwarf_P_Attribute dwarf_add_AT_flag_compat(
  Dwarf_P_Debug dbg,
  Dwarf_P_Die ownerDie,
  Dwarf_Half attr,
  Dwarf_Small flag,
  Dwarf_Error* error
) {
  Dwarf_P_Attribute attribute = nullptr;
  int res = dwarf_add_AT_flag_a(dbg, ownerDie, attr, flag, &attribute, error);
  if (res != DW_DLV_OK) {
    return nullptr;
  }

  return attribute;
}

Dwarf_P_Attribute dwarf_add_AT_unsigned_const_compat(
  Dwarf_P_Debug dbg,
  Dwarf_P_Die ownerDie,
  Dwarf_Half attr,
  Dwarf_Unsigned value,
  Dwarf_Error* error
) {
  Dwarf_P_Attribute attribute = nullptr;
  int res = dwarf_add_AT_unsigned_const_a(dbg, ownerDie, attr, value, &attribute, error);
  if (res != DW_DLV_OK) {
    return nullptr;
  }

  return attribute;
}

Dwarf_P_Fde dwarf_new_fde_compat(
  Dwarf_P_Debug producer,
  Dwarf_Error* error
) {
  Dwarf_P_Fde fde = nullptr;
  int res = dwarf_new_fde_a(producer, &fde, error);
  if (res != DW_DLV_OK) {
    return nullptr;
  }

  return fde;
}

Dwarf_Unsigned dwarf_add_frame_fde_compat(
  Dwarf_P_Debug dbg,
  Dwarf_P_Fde fde,
  Dwarf_P_Die die,
  Dwarf_Unsigned cie,
  Dwarf_Unsigned virtAddr,
  Dwarf_Unsigned codeLen,
  Dwarf_Unsigned symbolIndex,
  Dwarf_Error* error
) {
  Dwarf_Unsigned index = 0;
  int res = dwarf_add_frame_fde_c(dbg, fde, die, cie, virtAddr, codeLen, symbolIndex, 0, 0, &index, error);
  if (res != DW_DLV_OK) {
    return DW_DLV_NOCOUNT;
  }
  return index;
}

Dwarf_Unsigned dwarf_add_file_decl_compat(
  Dwarf_P_Debug dbg,
  char* name,
  Dwarf_Unsigned dirIndex,
  Dwarf_Unsigned lastMod,
  Dwarf_Unsigned length,
  Dwarf_Error* error
) {
  Dwarf_Unsigned index = 0;
  int res = dwarf_add_file_decl_a(dbg, name, dirIndex, lastMod, length, &index, error);
  if (res != DW_DLV_OK) {
    return DW_DLV_NOCOUNT;
  }
  return index;
}

Dwarf_Unsigned dwarf_lne_set_address_compat(
  Dwarf_P_Debug dbg,
  Dwarf_Addr addr,
  Dwarf_Unsigned fileIndex,
  Dwarf_Error* error
) {
  int res = dwarf_lne_set_address_a(dbg, addr, fileIndex, error);
  if (res != DW_DLV_OK) {
    return DW_DLV_NOCOUNT;
  }
  return 0;
}

Dwarf_Unsigned dwarf_add_line_entry_compat(
  Dwarf_P_Debug dbg,
  Dwarf_Unsigned fileIndex,
  Dwarf_Addr codeAddress,
  Dwarf_Unsigned lineNo,
  Dwarf_Signed colNo,
  Dwarf_Bool isStmtBegin,
  Dwarf_Bool isBBBegin,
  Dwarf_Error* error
) {
  int res = dwarf_add_line_entry_c(dbg, fileIndex, codeAddress, lineNo, colNo, isStmtBegin, isBBBegin, 0, 0, 0, 0, error);
  if (res != DW_DLV_OK) {
    return DW_DLV_NOCOUNT;
  }
  return 0;
}

Dwarf_Unsigned dwarf_lne_end_sequence_compat(
  Dwarf_P_Debug dbg,
  Dwarf_Addr endAddress,
  Dwarf_Error* error
) {
  int res = dwarf_lne_end_sequence_a(dbg, endAddress, error);
  if (res != DW_DLV_OK) {
    return DW_DLV_NOCOUNT;
  }
  return 0;
}

Dwarf_P_Expr dwarf_new_expr_compat(Dwarf_P_Debug producer, Dwarf_Error* error) {
  Dwarf_P_Expr expr = nullptr;
  int res = dwarf_new_expr_a(producer, &expr, error);
  if (res != DW_DLV_OK) {
    return nullptr;
  }
  return expr;
}

Dwarf_Unsigned dwarf_add_die_to_debug_compat(
  Dwarf_P_Debug dbg,
  Dwarf_P_Die die,
  Dwarf_Error* error
) {
  int res = dwarf_add_die_to_debug_a(dbg, die, error);
  if (res != DW_DLV_OK) {
    return DW_DLV_NOCOUNT;
  }
  return 0;
}

Dwarf_Unsigned dwarf_add_frame_cie_compat(
  Dwarf_P_Debug dbg,
  char* augmenter,
  Dwarf_Small codeAlign,
  Dwarf_Small dataAlign,
  Dwarf_Small returnReg,
  Dwarf_Ptr initBytes,
  Dwarf_Unsigned initNBytes,
  Dwarf_Error* error
) {
  Dwarf_Unsigned index = 0;
  int res = dwarf_add_frame_cie_a(dbg, augmenter, codeAlign, dataAlign, returnReg, initBytes, initNBytes, &index, error);
  if (res != DW_DLV_OK) {
    return DW_DLV_NOCOUNT;
  }
  return index;
}

Dwarf_Unsigned dwarf_transform_to_disk_form_compat(
  Dwarf_P_Debug dbg,
  Dwarf_Error* error
) {
#ifdef LIBDWARF_USE_NEW_PRODUCER_API
  Dwarf_Unsigned count = 0;
#else
  Dwarf_Signed count = 0;
#endif

  int res = dwarf_transform_to_disk_form_a(dbg, &count, error);
  if (res == DW_DLV_ERROR) {
    return DW_DLV_NOCOUNT;
  }

#ifdef LIBDWARF_USE_NEW_PRODUCER_API
  return count;
#else
  return static_cast<Dwarf_Unsigned>(count);
#endif
}

Dwarf_Ptr dwarf_get_section_bytes_compat(
  Dwarf_P_Debug dbg,
  Dwarf_Unsigned* sectionIndex,
  Dwarf_Unsigned* length,
  Dwarf_Error* error
) {
  Dwarf_Signed unused;
  Dwarf_Ptr bytes = nullptr;

#ifdef LIBDWARF_USE_NEW_PRODUCER_API
  Dwarf_Unsigned bcSectionIndex;
#else
  Dwarf_Signed bcSectionIndex;
#endif

  int res = dwarf_get_section_bytes_a(dbg, unused, &bcSectionIndex, length, &bytes, error);
  if (res != DW_DLV_OK) {
    return nullptr;
  }

#ifdef LIBDWARF_USE_NEW_PRODUCER_API
  *sectionIndex = bcSectionIndex;
#else
  *sectionIndex = static_cast<Dwarf_Unsigned>(bcSectionIndex);
#endif

  return bytes;
}

Dwarf_P_Die ElfWriter::addFunctionInfo(FunctionInfo* f, Dwarf_P_Die type) {
  Dwarf_Error error = 0;

  /* top level DIE for each function */
  Dwarf_P_Die func = dwarf_new_die_compat(m_dwarfProducer,
    DW_TAG_subprogram, nullptr, nullptr, nullptr, nullptr, &error);
  if (func == nullptr) {
    logError("unable to create child DIE");
    return nullptr;
  }

  Dwarf_Signed file;
  FileDB::iterator it = m_fileDB.find(f->file);
  /* if this function is from an unseen file, register file name
   * and get index to file name */
  if (it == m_fileDB.end()) {
    file = dwarf_add_file_decl_compat(m_dwarfProducer,
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
  at = dwarf_add_AT_name_compat(func, (char *)f->name.c_str(), &error);
  if (at == nullptr) {
    logError("unable to add name attribute to function");
    return nullptr;
  }

  /* Add lower PC bound to function DIE */
  at = dwarf_add_AT_targ_address_compat(m_dwarfProducer, func, DW_AT_low_pc,
    reinterpret_cast<Dwarf_Unsigned>(f->range.begin()), 0, &error);
  if (at == nullptr) {
    logError("unable to add low_pc attribute to function");
    return nullptr;
  }

  /* add upper PC bound to function DIE */
  at = dwarf_add_AT_targ_address_compat(m_dwarfProducer, func, DW_AT_high_pc,
    reinterpret_cast<Dwarf_Unsigned>(f->range.end()), 0, &error);
  if (at == nullptr) {
    logError("unable to add high_pc attribute to function");
    return nullptr;
  }

  /* register line number information for function:
   * 1. register start address */
  Dwarf_Unsigned u;
  u = dwarf_lne_set_address_compat(m_dwarfProducer,
    reinterpret_cast<Dwarf_Addr>(f->range.begin()), 0, &error);
  if (u != 0) {
    logError("unable to set line start address");
    return nullptr;
  }

  /* 2. register line number info for each tracelet in function */
  std::vector<LineEntry>::iterator it2;
  for (it2 = f->m_lineTable.begin(); it2 != f->m_lineTable.end(); it2++) {
    u = dwarf_add_line_entry_compat(m_dwarfProducer,
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
  u = dwarf_lne_end_sequence_compat(m_dwarfProducer,
    reinterpret_cast<Dwarf_Addr>(f->range.end()), &error);
  if (u != 0) {
    logError("unable to set line end address");
    return nullptr;
  }

  {
    /* 4. register frame base of function */
    Dwarf_P_Expr locExpr = dwarf_new_expr_compat(m_dwarfProducer, &error);
    if (locExpr == nullptr) {
      logError("unable to create new location expression");
      return nullptr;
    }

    u = dwarf_add_expr_gen_compat(locExpr, DW_OP_call_frame_cfa, 0, 0, &error);
    if (u == DW_DLV_NOCOUNT) {
      logError("unable to add subexpression to location expression");
      return nullptr;
    }

    u = dwarf_add_expr_gen_compat(locExpr, DW_OP_const1u, CFA_OFFSET, 0, &error);
    if (u == DW_DLV_NOCOUNT) {
      logError("unable to add subexpression to location expression");
      return nullptr;
    }

    u = dwarf_add_expr_gen_compat(locExpr, DW_OP_minus, 0, 0, &error);
    if (u == DW_DLV_NOCOUNT) {
      logError("unable to add subexpression to location expression");
      return nullptr;
    }

    Dwarf_P_Attribute frameBaseAttr = dwarf_add_AT_location_expr_compat(
      m_dwarfProducer, func, DW_AT_frame_base, locExpr, &error);
    if (frameBaseAttr == nullptr) {
      logError("unable to add frame_base attribute");
      return nullptr;
    }
  }

  /* 5. register all the named locals of function */
  Dwarf_P_Die lastLocal = nullptr;
  int i = 1;
  for (std::vector<std::string>::iterator it = f->m_namedLocals.begin();
       it != f->m_namedLocals.end(); it++) {
    Dwarf_P_Die localVar = dwarf_new_die_compat(m_dwarfProducer,
      DW_TAG_variable, nullptr, nullptr, nullptr, nullptr, &error);
    if (localVar == nullptr) {
      logError("unable to create new DIE for local variable");
      return nullptr;
    }

    /* Create location expression defined w.r.t DW_AT_frame_base */
    Dwarf_P_Expr locExpr = dwarf_new_expr_compat(m_dwarfProducer, &error);
    if (locExpr == nullptr) {
      logError("unable to create new location expression");
      return nullptr;
    }

    u = dwarf_add_expr_gen_compat(locExpr, DW_OP_fbreg,
      -(i * sizeof(TypedValue)), 0, &error);
    ++i;
    if (u == DW_DLV_NOCOUNT) {
      logError("unable to add subexpression to location expression");
      return nullptr;
    }

    Dwarf_P_Attribute locAttr = dwarf_add_AT_location_expr_compat(m_dwarfProducer,
      localVar, DW_AT_location, locExpr, &error);
    if (locAttr == nullptr) {
      logError("unable to add location attribute to local variable");
      return nullptr;
    }

    Dwarf_P_Attribute nameAttr = dwarf_add_AT_name_compat(localVar, const_cast<char *>(it->data()), &error);
    if (nameAttr == nullptr) {
      logError("unable to add name attribute to local variable");
      return nullptr;
    }

    Dwarf_P_Attribute varTypeAttr = dwarf_add_AT_reference_compat(m_dwarfProducer,
      localVar, DW_AT_type, type, &error);
    if (varTypeAttr == nullptr) {
      logError("unable to add type attribute to local variable DIE");
      return nullptr;
    }

    Dwarf_P_Die res = 0;
    if (lastLocal != nullptr) {
      res = dwarf_die_link_compat(localVar, nullptr, nullptr, lastLocal, nullptr, &error);
    } else {
      res = dwarf_die_link_compat(localVar, func, nullptr, nullptr, nullptr, &error);
    }
    if (res == nullptr) {
      logError("unable to link die");
      return nullptr;
    }
    lastLocal = localVar;
  }

  return func;
}

Dwarf_P_Die ElfWriter::makeLocalTypeDie() {
  Dwarf_Error error = 0;
  Dwarf_P_Die typedValueType = dwarf_new_die_compat(m_dwarfProducer,
    DW_TAG_structure_type, nullptr, nullptr, nullptr, nullptr, &error);
  if (typedValueType == nullptr) {
    logError("unable to create new DIE for TypedValue type");
    return nullptr;
  }

  /* hard coding the name of 'HPHP::TypedValue' */
  Dwarf_P_Attribute at;
  at = dwarf_add_AT_name_compat(typedValueType, "HPHP::TypedValue", &error);
  if (at == nullptr) {
    logError("unable to add name attribute to TypedValue type DIE");
    return nullptr;
  }

  at = dwarf_add_AT_flag_compat(m_dwarfProducer,
    typedValueType, DW_AT_declaration, 1, &error);
  if (at == nullptr) {
    logError("unable to add declaration attribute to TypedValue type DIE");
    return nullptr;
  }

  at = dwarf_add_AT_unsigned_const_compat(m_dwarfProducer,
    typedValueType, DW_AT_byte_size, sizeof(TypedValue), &error);
  if (at == nullptr) {
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
  Dwarf_P_Die codeUnit = dwarf_new_die_compat(m_dwarfProducer,
    DW_TAG_compile_unit, nullptr, nullptr, nullptr, nullptr, &error);
  if (codeUnit == nullptr) {
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
  linkRes = dwarf_die_link_compat(type, codeUnit, nullptr, nullptr, nullptr, &error);
  if (linkRes == nullptr) {
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
      linkRes = dwarf_die_link_compat(func, nullptr, nullptr, lastChild, nullptr, &error);
    } else {
      linkRes = dwarf_die_link_compat(func, codeUnit, nullptr, nullptr, nullptr, &error);
    }
    if (linkRes == nullptr) {
      logError("unable to link die");
      return false;
    }
    lastChild = func;
  }

  /* register top level DIE */
  Dwarf_Unsigned res = dwarf_add_die_to_debug_compat(m_dwarfProducer,
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
  Dwarf_Signed cie_index = dwarf_add_frame_cie_compat(
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
    Dwarf_P_Fde fde = dwarf_new_fde_compat(m_dwarfProducer, &error);
    if (fde == nullptr) {
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
    Dwarf_Unsigned fde_index = dwarf_add_frame_fde_compat(
      m_dwarfProducer, fde, 0, cie_index,
      (Dwarf_Unsigned)((*it)->range.begin()),
      (*it)->range.size(),
      0, &error);
    if (fde_index == DW_DLV_NOCOUNT) {
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
  Dwarf_Unsigned sections = dwarf_transform_to_disk_form_compat(m_dwarfProducer, 0);

  Dwarf_Signed i = 0;
  Dwarf_Unsigned elf_section_index = 0;
  Dwarf_Unsigned length = 0;

  for (i = 0; i < sections; i++) {
    Dwarf_Ptr bytes = dwarf_get_section_bytes_compat(
      m_dwarfProducer, &elf_section_index, &length, 0);

    if (bytes == nullptr) {
      logError("Unable to create section");
      return false;
    }

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
    dwarf_producer_finish_a(m_dwarfProducer, 0);
}

}
}
