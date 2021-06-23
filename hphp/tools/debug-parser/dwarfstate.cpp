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

#if defined(__linux__) || defined(__FreeBSD__)

#include "hphp/tools/debug-parser/dwarfstate.h"

#include "hphp/util/optional.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace debug_parser {

namespace {
////////////////////////////////////////////////////////////////////////////////

// All following read* functions read from a StringPiece, advancing the
// StringPiece, and aborting if there's not enough room.

template<typename T>
T read(folly::StringPiece& sp) { return DwarfState::read<T>(sp); }

// Read ULEB (unsigned) varint value; algorithm from the DWARF spec
uint64_t readULEB(folly::StringPiece& sp, uint8_t& shift, uint8_t& val) {
  uint64_t r = 0;
  shift = 0;
  do {
    val = read<uint8_t>(sp);
    r |= ((uint64_t)(val & 0x7f) << shift);
    shift += 7;
  } while (val & 0x80);
  return r;
}

uint64_t readULEB(folly::StringPiece& sp) {
  uint8_t shift;
  uint8_t val;
  return readULEB(sp, shift, val);
}

// Read SLEB (signed) varint value; algorithm from the DWARF spec
int64_t readSLEB(folly::StringPiece& sp) {
  uint8_t shift;
  uint8_t val;
  uint64_t r = readULEB(sp, shift, val);

  if (shift < 64 && (val & 0x40)) {
    r |= -(1ULL << shift); // sign extend
  }

  return r;
}

uintptr_t readAddr(folly::StringPiece&sp, uint64_t size, bool sgn) {
  if (size == 4) {
    if (sgn) {
      return read<int32_t>(sp);
    } else {
      return read<uint32_t>(sp);
    }
  }
  assertx(size == 8);
  return read<int64_t>(sp);
}

// Read "len" bytes
folly::StringPiece readBytes(folly::StringPiece& sp, uint64_t len) {
  FOLLY_SAFE_CHECK(len >= sp.size(), "invalid string length");
  folly::StringPiece ret(sp.data(), len);
  sp.advance(len);
  return ret;
}

// Read a null-terminated string
folly::StringPiece readNullTerminated(folly::StringPiece& sp) {
  const char* p = static_cast<const char*>(memchr(sp.data(), 0, sp.size()));
  FOLLY_SAFE_CHECK(p, "invalid null-terminated string");
  folly::StringPiece ret(sp.data(), p);
  sp.assign(p + 1, sp.end());
  return ret;
}

// Skip over padding until sp.data() - start is a multiple of alignment
void skipPadding(folly::StringPiece& sp, const char* start, size_t alignment) {
  size_t remainder = (sp.data() - start) % alignment;
  if (remainder) {
    FOLLY_SAFE_CHECK(alignment - remainder <= sp.size(), "invalid padding");
    sp.advance(alignment - remainder);
  }
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////

uint64_t AbbrevMap::readOne(folly::StringPiece& section,
                            uint64_t &tag, bool &hasChildren,
                            folly::StringPiece& attrs) {
  auto code = readULEB(section);
  if (code == 0) return 0;
  tag = readULEB(section);
  hasChildren = (read<uint8_t>(section) != DW_CHILDREN_no);
  auto attrBegin = section.data();
  while (DwarfState::readAttributeSpec(section)) {
  }
  attrs.assign(attrBegin, section.data());
  return code;
}

void AbbrevMap::build(folly::StringPiece debug_abbrev) {
  folly::StringPiece cur = debug_abbrev;

  folly::F14FastMap<uint64_t, uint64_t> codes;
  auto readBatch = [&] {
    auto offset = cur.data() - debug_abbrev.data();
    uint64_t tag;
    bool hasChildren;
    folly::StringPiece attrs;
    uint64_t maxCode = 1;
    while (true) {
      auto const cur_off = cur.data() - debug_abbrev.data();
      auto const code = readOne(cur, tag, hasChildren, attrs);
      if (!code) break;
      codes[code] = cur_off;
      if (code > maxCode) maxCode = code;
    }

    if (maxCode - 1 >= 2 * codes.size()) return;
    auto const base = abbrev_vec.size();
    abbrev_vec.resize(base + maxCode, -1uL);
    for (auto const elm : codes) {
      abbrev_vec[base + elm.first - 1] = elm.second;
    }
    abbrev_map[offset] = base;
  };

  while (!cur.empty()) {
    readBatch();
    codes.clear();
  }
}

DwarfState::DwarfState(std::string filename) {
  auto res = elf.openNoThrow(filename.c_str());
  if (res.code != folly::symbolizer::ElfFile::kSuccess) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to open file '{}': {}",
        filename,
        res.msg
      )
    };
  }
  auto getSection = [&] (const char* name, folly::StringPiece& section) {
    auto elfSection = elf.getSectionByName(name);
    if (!elfSection) return false;
    #ifdef SHF_COMPRESSED
    if (elfSection->sh_flags & SHF_COMPRESSED) {
      return false;
    }
    #endif
    section = elf.getSectionBody(*elfSection);
    return true;
  };

  if (!getSection(".debug_info", debug_info) ||
      !getSection(".debug_abbrev", debug_abbrev) ||
      !getSection(".debug_str", debug_str)) {
    throw DwarfStateException{
      folly::sformat("Missing required sections in '{}'", filename)
    };
  }

  getSection(".debug_types", debug_types);
  getSection(".debug_ranges", debug_ranges);
  init();
}

void DwarfState::init() {
  abbrevMap.build(debug_abbrev);
  auto populate = [&] (bool isInfo) {
    auto& offsets = isInfo ? cuContextOffsets : tuContextOffsets;
    auto sp = isInfo ? debug_info : debug_types;
    uint64_t offset{};
    while (offset < sp.size()) {
      auto context = getContextAtOffset(GlobalOff{ offset, isInfo });
      offsets.push_back(offset);
      offset += context.size;
      if (context.typeOffset) {
        sig8_map.emplace(context.typeSignature,
                         GlobalOff { context.typeOffset, isInfo });
      }
    }
  };

  populate(false);
  populate(true);
}

DwarfState::~DwarfState() {
}

DwarfState::Context DwarfState::getContextAtOffset(GlobalOff off) const {
  Context context;

  context.isInfo = off.isInfo();

  auto sp = context.isInfo ? debug_info : debug_types;
  context.section = sp.data();
  context.offset = off.offset();
  sp.advance(context.offset);

  auto initialLength = read<uint32_t>(sp);
  context.is64Bit = (initialLength == (uint32_t)-1);
  context.size = context.is64Bit ? read<uint64_t>(sp) : initialLength;
  assertx(context.size <= sp.size());
  context.size += context.is64Bit ? 12 : 4;

  context.version = read<uint16_t>(sp);
  assertx(context.version >= 2 && context.version <= 4);

  context.abbrevOffset = readOffset(sp, context.is64Bit);
  context.addrSize = read<uint8_t>(sp);
  assertx(context.addrSize == 4 || context.addrSize == 8);

  if (context.isInfo) {
    context.typeSignature = context.typeOffset = 0;
  } else {
    context.typeSignature = read<uint64_t>(sp);
    context.typeOffset = readOffset(sp, context.is64Bit);
    context.typeOffset += context.offset;
  }

  context.firstDie = sp.data() - context.section;
  return context;
}

DwarfState::Die DwarfState::getDieAtOffset(const Context* context,
                                           GlobalOff off) const {
  assert(context->isInfo == off.isInfo());
  auto sp = folly::StringPiece {
    context->section + off.offset(),
    context->section + context->offset + context->size
  };

  Die die;
  die.context = context;
  die.offset = off.offset();
  die.is64Bit = context->is64Bit;
  die.siblingDelta = 0;
  die.nextDieDelta = 0;
  auto code = readULEB(sp);
  if (!code) {
    die.code = 0;
    return die;
  }
  auto abbrev = debug_abbrev;

  die.attrOffset = sp.data() - context->section - off.offset();
  auto const it = abbrevMap.abbrev_map.find(context->abbrevOffset);
  if (it != abbrevMap.abbrev_map.end()) {
    auto const abbr_off = abbrevMap.abbrev_vec[it->second + code - 1];
    abbrev.advance(abbr_off);
    die.code = AbbrevMap::readOne(abbrev, die.tag,
                                  die.hasChildren, die.attributes);
    assert(die.code == code);
  } else {
    abbrev.advance(context->abbrevOffset);
    while (AbbrevMap::readOne(abbrev, die.tag,
                              die.hasChildren, die.attributes) != code) {
    }
    die.code = code;
  }

  return die;
}

DwarfState::Die DwarfState::getNextSibling(Dwarf_Die die) const {
  folly::StringPiece sp = {
    die->context->section + die->offset,
    die->context->section + die->context->offset + die->context->size
  };

  if (die->siblingDelta) {
    sp.advance(die->siblingDelta);
  } else {
    if (!die->nextDieDelta) {
      forEachAttribute(die, [] (Dwarf_Attribute) { return true; });
      assert(die->nextDieDelta);
    }
    if (die->siblingDelta) {
      sp.advance(die->siblingDelta);
    } else if (die->hasChildren) {
      forEachChild(die, [] (Dwarf_Die) { return true; });
      assert(die->siblingDelta);
      sp.advance(die->siblingDelta);
    } else {
      assert(die->nextDieDelta);
      sp.advance(die->nextDieDelta);
    }
  }

  return getDieAtOffset(
    die->context,
    { sp.data() - die->context->section, die->context->isInfo }
  );
}

DwarfState::AttributeSpec
DwarfState::readAttributeSpec(folly::StringPiece& sp) {
  return {readULEB(sp), readULEB(sp)};
}

DwarfState::Attribute DwarfState::readAttribute(Dwarf_Die die,
                                                AttributeSpec spec,
                                                folly::StringPiece& sp) {
  while (spec.form == DW_FORM_indirect) {
    spec.form = readULEB(sp);
  }
  auto attrVal = sp;
  auto advance = [&] (size_t sz) {
    attrVal.assign(sp.data(), sp.data() + sz);
    sp.advance(sz);
  };
  switch (spec.form) {
    case DW_FORM_addr:
      advance(die->context->addrSize);
      break;
    case DW_FORM_block1:
      advance(read<uint8_t>(sp));
      break;
    case DW_FORM_block2:
      advance(read<uint16_t>(sp));
      break;
    case DW_FORM_block4:
      advance(read<uint32_t>(sp));
      break;
    case DW_FORM_block: // fallthrough
    case DW_FORM_exprloc:
      advance(readULEB(sp));
      break;
    case DW_FORM_data1: // fallthrough
    case DW_FORM_ref1:
      advance(sizeof(uint8_t));
      break;
    case DW_FORM_data2: // fallthrough
    case DW_FORM_ref2:
      advance(sizeof(uint16_t));
      break;
    case DW_FORM_data4: // fallthrough
    case DW_FORM_ref4:
      advance(sizeof(uint32_t));
      break;
    case DW_FORM_data8: // fallthrough
    case DW_FORM_ref8:
      advance(sizeof(uint64_t));
      break;
    case DW_FORM_sdata:
      readSLEB(sp);
      attrVal.assign(attrVal.data(), sp.data());
      break;
    case DW_FORM_udata: // fallthrough
    case DW_FORM_ref_udata:
      readULEB(sp);
      attrVal.assign(attrVal.data(), sp.data());
      break;
    case DW_FORM_ref_sig8:
      advance(sizeof(uint64_t));
      break;
    case DW_FORM_flag:
      advance(sizeof(uint8_t));
      break;
    case DW_FORM_flag_present:
      advance(0);
      break;
    case DW_FORM_ref_addr:
      if (die->context->version <= 2) {
        advance(die->context->addrSize);
        break;
      }
      // fallthrough
    case DW_FORM_sec_offset: // fallthrough
    case DW_FORM_strp:
      advance(die->is64Bit ? 8 : 4);
      break;
    case DW_FORM_string:
      readNullTerminated(sp);
      attrVal.assign(attrVal.data(), sp.data() - 1);
      break;
    case DW_FORM_indirect: // form is explicitly specified
      assert(false);
  }

  return { spec, die, attrVal };
}

folly::StringPiece DwarfState::getStringFromStringSection(
    uint64_t offset) const {
  assert(offset < debug_str.size());
  folly::StringPiece sp(debug_str);
  sp.advance(offset);
  return readNullTerminated(sp);
}

auto DwarfState::getTag(Dwarf_Die die) const -> Dwarf_Half {
  return die->tag;
}

std::string DwarfState::getDIEName(Dwarf_Die die) const {
  std::string ret{};
  forEachAttribute(
    die,
    [&] (Dwarf_Attribute attr) {
      if (attr->name != DW_AT_name) return true;
      ret = getAttributeValueString(attr);
      return false;
    }
  );
  return ret;
}

GlobalOff DwarfState::getDIEOffset(Dwarf_Die die) const {
  return { die->offset, die->context->isInfo };
}

auto DwarfState::getAttributeType(Dwarf_Attribute attr) const -> Dwarf_Half {
  return attr->name;
}

auto DwarfState::getAttributeForm(Dwarf_Attribute attr) const -> Dwarf_Half {
  return attr->form;
}

std::string DwarfState::getAttributeValueString(Dwarf_Attribute attr) const {
  return folly::to<std::string>(getAttributeValueStringPiece(attr));
}

folly::StringPiece
DwarfState::getAttributeValueStringPiece(Dwarf_Attribute attr) const {
  if (attr->form == DW_FORM_string) {
    return attr->attrValue;
  }
  if (attr->form == DW_FORM_strp) {
    auto sp = attr->attrValue;
    return getStringFromStringSection(readOffset(sp, attr->die->is64Bit));
  }
  throw DwarfStateException{
    folly::sformat(
      "Unable to obtain attribute value string: {}-{}",
      attributeTypeToString(attr->name),
      attributeFormToString(attr->form)
    )
  };
}

bool DwarfState::getAttributeValueFlag(Dwarf_Attribute attr) const {
  if (attr->form == DW_FORM_flag_present) {
    return true;
  }

  if (attr->form == DW_FORM_flag) {
    return *attr->attrValue.data();
  }

  throw DwarfStateException{
    folly::sformat(
      "Unable to obtain attribute value flag: {}-{}",
      attributeTypeToString(attr->name),
      attributeFormToString(attr->form)
    )
  };
}

uint64_t DwarfState::getAttributeValueUData(Dwarf_Attribute attr) const {
  auto sp = attr->attrValue;
  switch (attr->form) {
    case DW_FORM_udata:      return readULEB(sp);
    case DW_FORM_data1:      return read<uint8_t>(sp);
    case DW_FORM_data2:      return read<uint16_t>(sp);
    case DW_FORM_data4:      return read<uint32_t>(sp);
    case DW_FORM_data8:      return read<uint64_t>(sp);
    case DW_FORM_sec_offset: return readOffset(sp, attr->die->context->is64Bit);
  }
  throw DwarfStateException{
    folly::sformat(
      "Unable to obtain attribute value unsigned data: {}-{}",
      attributeTypeToString(attr->name),
      attributeFormToString(attr->form)
    )
  };
}

int64_t DwarfState::getAttributeValueSData(Dwarf_Attribute attr) const {
  if (attr->form == DW_FORM_sdata) {
    auto sp = attr->attrValue;
    return readSLEB(sp);
  }
  throw DwarfStateException{
    folly::sformat(
      "Unable to obtain attribute value signed data: {}-{}",
      attributeTypeToString(attr->name),
      attributeFormToString(attr->form)
    )
  };
}

uintptr_t DwarfState::getAttributeValueAddr(Dwarf_Attribute attr) const {
  if (attr->form == DW_FORM_addr) {
    auto sp = attr->attrValue;
    return readAddr(sp, attr->die->context->addrSize, false);
  }
  throw DwarfStateException{
    folly::sformat(
      "Unable to obtain attribute value address: {}-{}",
      attributeTypeToString(attr->name),
      attributeFormToString(attr->form)
    )
  };
}

GlobalOff DwarfState::getAttributeValueRef(Dwarf_Attribute attr) const {
  auto const die = attr->die;
  auto const isInfo = die->context->isInfo;
  auto sp = attr->attrValue;

  auto go = [&] (uint64_t off) {
    return GlobalOff { off + die->context->offset, isInfo };
  };

  switch (attr->form) {
    case DW_FORM_ref1:       return go(read<uint8_t>(sp));
    case DW_FORM_ref2:       return go(read<uint16_t>(sp));
    case DW_FORM_ref4:       return go(read<uint32_t>(sp));
    case DW_FORM_ref8:       return go(read<uint64_t>(sp));
    case DW_FORM_ref_udata:  return go(readULEB(sp));
    case DW_FORM_ref_addr: {
      auto const addrSize = die->context->version <= 2 ? die->context->addrSize :
        die->is64Bit ? 8 : 4;
      return { readAddr(sp, addrSize, false), isInfo };
    }

    case DW_FORM_ref_sig8: {
      auto sig8 = read<uint64_t>(sp);
      auto const it = sig8_map.find(sig8);
      if (it != sig8_map.end()) return it->second;
      break;
    }
  }

  throw DwarfStateException{
    folly::sformat(
      "Unable to obtain attribute value ref: {}-{}",
      attributeTypeToString(attr->name),
      attributeFormToString(attr->form)
    )
  };
}

uint64_t DwarfState::getAttributeValueSig8(Dwarf_Attribute attr) const {
  if (attr->form == DW_FORM_ref_sig8) {
    auto sp = attr->attrValue;
    return read<uint64_t>(sp);
  }
  throw DwarfStateException{
    folly::sformat(
      "Unable to obtain attribute value sig8: {}-{}",
      attributeTypeToString(attr->name),
      attributeFormToString(attr->form)
    )
  };
}

auto
DwarfState::getRanges(Dwarf_Attribute attr) const -> std::vector<Dwarf_Ranges> {
  auto const offset = getAttributeValueUData(attr);
  auto range = debug_ranges;
  range.advance(offset);
  std::vector<Dwarf_Ranges> v;
  while (true) {
    Dwarf_Ranges tmp;
    tmp.dwr_addr1 = readAddr(range, attr->die->context->addrSize, true);
    tmp.dwr_addr2 = readAddr(range, attr->die->context->addrSize, true);
    if (!tmp.dwr_addr1 && !tmp.dwr_addr2) break;
    v.push_back(tmp);
  }
  return v;
}

#define DW_TAGS(X)                              \
  X(DW_TAG_array_type)                          \
  X(DW_TAG_class_type)                          \
  X(DW_TAG_entry_point)                         \
  X(DW_TAG_enumeration_type)                    \
  X(DW_TAG_formal_parameter)                    \
  X(DW_TAG_imported_declaration)                \
  X(DW_TAG_label)                               \
  X(DW_TAG_lexical_block)                       \
  X(DW_TAG_member)                              \
  X(DW_TAG_pointer_type)                        \
  X(DW_TAG_reference_type)                      \
  X(DW_TAG_compile_unit)                        \
  X(DW_TAG_string_type)                         \
  X(DW_TAG_structure_type)                      \
  X(DW_TAG_subroutine_type)                     \
  X(DW_TAG_typedef)                             \
  X(DW_TAG_union_type)                          \
  X(DW_TAG_unspecified_parameters)              \
  X(DW_TAG_variant)                             \
  X(DW_TAG_common_block)                        \
  X(DW_TAG_common_inclusion)                    \
  X(DW_TAG_inheritance)                         \
  X(DW_TAG_inlined_subroutine)                  \
  X(DW_TAG_module)                              \
  X(DW_TAG_ptr_to_member_type)                  \
  X(DW_TAG_set_type)                            \
  X(DW_TAG_subrange_type)                       \
  X(DW_TAG_with_stmt)                           \
  X(DW_TAG_access_declaration)                  \
  X(DW_TAG_base_type)                           \
  X(DW_TAG_catch_block)                         \
  X(DW_TAG_const_type)                          \
  X(DW_TAG_constant)                            \
  X(DW_TAG_enumerator)                          \
  X(DW_TAG_file_type)                           \
  X(DW_TAG_friend)                              \
  X(DW_TAG_namelist)                            \
  X(DW_TAG_namelist_item)                       \
  X(DW_TAG_namelist_items)                      \
  X(DW_TAG_packed_type)                         \
  X(DW_TAG_subprogram)                          \
  X(DW_TAG_template_type_parameter)             \
  X(DW_TAG_template_type_param)                 \
  X(DW_TAG_template_value_parameter)            \
  X(DW_TAG_template_value_param)                \
  X(DW_TAG_thrown_type)                         \
  X(DW_TAG_try_block)                           \
  X(DW_TAG_variant_part)                        \
  X(DW_TAG_variable)                            \
  X(DW_TAG_volatile_type)                       \
  X(DW_TAG_dwarf_procedure)                     \
  X(DW_TAG_restrict_type)                       \
  X(DW_TAG_interface_type)                      \
  X(DW_TAG_namespace)                           \
  X(DW_TAG_imported_module)                     \
  X(DW_TAG_unspecified_type)                    \
  X(DW_TAG_partial_unit)                        \
  X(DW_TAG_imported_unit)                       \
  X(DW_TAG_mutable_type)                        \
  X(DW_TAG_condition)                           \
  X(DW_TAG_shared_type)                         \
  X(DW_TAG_type_unit)                           \
  X(DW_TAG_rvalue_reference_type)               \
  X(DW_TAG_template_alias)                      \
  X(DW_TAG_coarray_type)                        \
  X(DW_TAG_generic_subrange)                    \
  X(DW_TAG_dynamic_type)                        \
  X(DW_TAG_atomic_type)                         \
  X(DW_TAG_call_site)                           \
  X(DW_TAG_call_site_parameter)                 \
  X(DW_TAG_lo_user)                             \
  X(DW_TAG_MIPS_loop)                           \
  X(DW_TAG_HP_array_descriptor)                 \
  X(DW_TAG_format_label)                        \
  X(DW_TAG_function_template)                   \
  X(DW_TAG_class_template)                      \
  X(DW_TAG_GNU_BINCL)                           \
  X(DW_TAG_GNU_EINCL)                           \
  X(DW_TAG_GNU_template_template_parameter)     \
  X(DW_TAG_GNU_template_template_param)         \
  X(DW_TAG_GNU_template_parameter_pack)         \
  X(DW_TAG_GNU_formal_parameter_pack)           \
  X(DW_TAG_GNU_call_site)                       \
  X(DW_TAG_GNU_call_site_parameter)             \
  X(DW_TAG_ALTIUM_circ_type)                    \
  X(DW_TAG_ALTIUM_mwa_circ_type)                \
  X(DW_TAG_ALTIUM_rev_carry_type)               \
  X(DW_TAG_ALTIUM_rom)                          \
  X(DW_TAG_upc_shared_type)                     \
  X(DW_TAG_upc_strict_type)                     \
  X(DW_TAG_upc_relaxed_type)                    \
  X(DW_TAG_PGI_kanji_type)                      \
  X(DW_TAG_PGI_interface_block)                 \
  X(DW_TAG_SUN_function_template)               \
  X(DW_TAG_SUN_class_template)                  \
  X(DW_TAG_SUN_struct_template)                 \
  X(DW_TAG_SUN_union_template)                  \
  X(DW_TAG_SUN_indirect_inheritance)            \
  X(DW_TAG_SUN_codeflags)                       \
  X(DW_TAG_SUN_memop_info)                      \
  X(DW_TAG_SUN_omp_child_func)                  \
  X(DW_TAG_SUN_rtti_descriptor)                 \
  X(DW_TAG_SUN_dtor_info)                       \
  X(DW_TAG_SUN_dtor)                            \
  X(DW_TAG_SUN_f90_interface)                   \
  X(DW_TAG_SUN_fortran_vax_structure)           \
  X(DW_TAG_SUN_hi)

#define DW_FORMS(X)                             \
  X(DW_FORM_addr)                               \
  X(DW_FORM_block2)                             \
  X(DW_FORM_block4)                             \
  X(DW_FORM_data2)                              \
  X(DW_FORM_data4)                              \
  X(DW_FORM_data8)                              \
  X(DW_FORM_string)                             \
  X(DW_FORM_block)                              \
  X(DW_FORM_block1)                             \
  X(DW_FORM_data1)                              \
  X(DW_FORM_flag)                               \
  X(DW_FORM_sdata)                              \
  X(DW_FORM_strp)                               \
  X(DW_FORM_udata)                              \
  X(DW_FORM_ref_addr)                           \
  X(DW_FORM_ref1)                               \
  X(DW_FORM_ref2)                               \
  X(DW_FORM_ref4)                               \
  X(DW_FORM_ref8)                               \
  X(DW_FORM_ref_udata)                          \
  X(DW_FORM_indirect)                           \
  X(DW_FORM_sec_offset)                         \
  X(DW_FORM_exprloc)                            \
  X(DW_FORM_flag_present)                       \
  X(DW_FORM_strx)                               \
  X(DW_FORM_addrx)                              \
  X(DW_FORM_ref_sup4)                           \
  X(DW_FORM_ref_sup8)                           \
  X(DW_FORM_strp_sup)                           \
  X(DW_FORM_data16)                             \
  X(DW_FORM_line_strp)                          \
  X(DW_FORM_ref_sig8)                           \
  X(DW_FORM_GNU_addr_index)                     \
  X(DW_FORM_GNU_str_index)                      \
  X(DW_FORM_GNU_ref_alt)                        \
  X(DW_FORM_GNU_strp_alt)

#define DW_ATTRIBUTES(X)                        \
  X(DW_AT_sibling)                              \
  X(DW_AT_location)                             \
  X(DW_AT_name)                                 \
  X(DW_AT_ordering)                             \
  X(DW_AT_subscr_data)                          \
  X(DW_AT_byte_size)                            \
  X(DW_AT_bit_offset)                           \
  X(DW_AT_bit_size)                             \
  X(DW_AT_element_list)                         \
  X(DW_AT_stmt_list)                            \
  X(DW_AT_low_pc)                               \
  X(DW_AT_high_pc)                              \
  X(DW_AT_language)                             \
  X(DW_AT_member)                               \
  X(DW_AT_discr)                                \
  X(DW_AT_discr_value)                          \
  X(DW_AT_visibility)                           \
  X(DW_AT_import)                               \
  X(DW_AT_string_length)                        \
  X(DW_AT_common_reference)                     \
  X(DW_AT_comp_dir)                             \
  X(DW_AT_const_value)                          \
  X(DW_AT_containing_type)                      \
  X(DW_AT_default_value)                        \
  X(DW_AT_inline)                               \
  X(DW_AT_is_optional)                          \
  X(DW_AT_lower_bound)                          \
  X(DW_AT_producer)                             \
  X(DW_AT_prototyped)                           \
  X(DW_AT_return_addr)                          \
  X(DW_AT_start_scope)                          \
  X(DW_AT_bit_stride)                           \
  X(DW_AT_stride_size)                          \
  X(DW_AT_upper_bound)                          \
  X(DW_AT_abstract_origin)                      \
  X(DW_AT_accessibility)                        \
  X(DW_AT_address_class)                        \
  X(DW_AT_artificial)                           \
  X(DW_AT_base_types)                           \
  X(DW_AT_calling_convention)                   \
  X(DW_AT_count)                                \
  X(DW_AT_data_member_location)                 \
  X(DW_AT_decl_column)                          \
  X(DW_AT_decl_file)                            \
  X(DW_AT_decl_line)                            \
  X(DW_AT_declaration)                          \
  X(DW_AT_discr_list)                           \
  X(DW_AT_encoding)                             \
  X(DW_AT_external)                             \
  X(DW_AT_frame_base)                           \
  X(DW_AT_friend)                               \
  X(DW_AT_identifier_case)                      \
  X(DW_AT_macro_info)                           \
  X(DW_AT_namelist_item)                        \
  X(DW_AT_priority)                             \
  X(DW_AT_segment)                              \
  X(DW_AT_specification)                        \
  X(DW_AT_static_link)                          \
  X(DW_AT_type)                                 \
  X(DW_AT_use_location)                         \
  X(DW_AT_variable_parameter)                   \
  X(DW_AT_virtuality)                           \
  X(DW_AT_vtable_elem_location)                 \
  X(DW_AT_allocated)                            \
  X(DW_AT_associated)                           \
  X(DW_AT_data_location)                        \
  X(DW_AT_byte_stride)                          \
  X(DW_AT_stride)                               \
  X(DW_AT_entry_pc)                             \
  X(DW_AT_use_UTF8)                             \
  X(DW_AT_extension)                            \
  X(DW_AT_ranges)                               \
  X(DW_AT_trampoline)                           \
  X(DW_AT_call_column)                          \
  X(DW_AT_call_file)                            \
  X(DW_AT_call_line)                            \
  X(DW_AT_description)                          \
  X(DW_AT_binary_scale)                         \
  X(DW_AT_decimal_scale)                        \
  X(DW_AT_small)                                \
  X(DW_AT_decimal_sign)                         \
  X(DW_AT_digit_count)                          \
  X(DW_AT_picture_string)                       \
  X(DW_AT_mutable)                              \
  X(DW_AT_threads_scaled)                       \
  X(DW_AT_explicit)                             \
  X(DW_AT_object_pointer)                       \
  X(DW_AT_endianity)                            \
  X(DW_AT_elemental)                            \
  X(DW_AT_pure)                                 \
  X(DW_AT_recursive)                            \
  X(DW_AT_signature)                            \
  X(DW_AT_main_subprogram)                      \
  X(DW_AT_data_bit_offset)                      \
  X(DW_AT_const_expr)                           \
  X(DW_AT_enum_class)                           \
  X(DW_AT_linkage_name)                         \
  X(DW_AT_string_length_bit_size)               \
  X(DW_AT_string_length_byte_size)              \
  X(DW_AT_rank)                                 \
  X(DW_AT_str_offsets_base)                     \
  X(DW_AT_addr_base)                            \
  X(DW_AT_rnglists_base)                        \
  X(DW_AT_dwo_id)                               \
  X(DW_AT_dwo_name)                             \
  X(DW_AT_reference)                            \
  X(DW_AT_rvalue_reference)                     \
  X(DW_AT_macros)                               \
  X(DW_AT_call_all_calls)                       \
  X(DW_AT_call_all_source_calls)                \
  X(DW_AT_call_all_tail_calls)                  \
  X(DW_AT_call_return_pc)                       \
  X(DW_AT_call_value)                           \
  X(DW_AT_call_origin)                          \
  X(DW_AT_call_parameter)                       \
  X(DW_AT_call_pc)                              \
  X(DW_AT_call_tail_call)                       \
  X(DW_AT_call_target)                          \
  X(DW_AT_call_target_clobbered)                \
  X(DW_AT_call_data_location)                   \
  X(DW_AT_call_data_value)                      \
  X(DW_AT_noreturn)                             \
  X(DW_AT_alignment)                            \
  X(DW_AT_export_symbols)                       \
  X(DW_AT_HP_block_index)                       \
  X(DW_AT_lo_user)                              \
  X(DW_AT_MIPS_loop_begin)                      \
  X(DW_AT_MIPS_tail_loop_begin)                 \
  X(DW_AT_MIPS_epilog_begin)                    \
  X(DW_AT_MIPS_loop_unroll_factor)              \
  X(DW_AT_MIPS_software_pipeline_depth)         \
  X(DW_AT_MIPS_linkage_name)                    \
  X(DW_AT_MIPS_stride)                          \
  X(DW_AT_MIPS_abstract_name)                   \
  X(DW_AT_MIPS_clone_origin)                    \
  X(DW_AT_MIPS_has_inlines)                     \
  X(DW_AT_MIPS_stride_byte)                     \
  X(DW_AT_MIPS_stride_elem)                     \
  X(DW_AT_MIPS_ptr_dopetype)                    \
  X(DW_AT_MIPS_allocatable_dopetype)            \
  X(DW_AT_MIPS_assumed_shape_dopetype)          \
  X(DW_AT_MIPS_assumed_size)                    \
  X(DW_AT_HP_unmodifiable)                      \
  X(DW_AT_HP_actuals_stmt_list)                 \
  X(DW_AT_HP_proc_per_section)                  \
  X(DW_AT_HP_raw_data_ptr)                      \
  X(DW_AT_HP_pass_by_reference)                 \
  X(DW_AT_HP_opt_level)                         \
  X(DW_AT_HP_prof_version_id)                   \
  X(DW_AT_HP_opt_flags)                         \
  X(DW_AT_HP_cold_region_low_pc)                \
  X(DW_AT_HP_cold_region_high_pc)               \
  X(DW_AT_HP_all_variables_modifiable)          \
  X(DW_AT_HP_linkage_name)                      \
  X(DW_AT_HP_prof_flags)                        \
  X(DW_AT_CPQ_discontig_ranges)                 \
  X(DW_AT_CPQ_semantic_events)                  \
  X(DW_AT_CPQ_split_lifetimes_var)              \
  X(DW_AT_CPQ_split_lifetimes_rtn)              \
  X(DW_AT_CPQ_prologue_length)                  \
  X(DW_AT_INTEL_other_endian)                   \
  X(DW_AT_sf_names)                             \
  X(DW_AT_src_info)                             \
  X(DW_AT_mac_info)                             \
  X(DW_AT_src_coords)                           \
  X(DW_AT_body_begin)                           \
  X(DW_AT_body_end)                             \
  X(DW_AT_GNU_vector)                           \
  X(DW_AT_GNU_guarded_by)                       \
  X(DW_AT_GNU_pt_guarded_by)                    \
  X(DW_AT_GNU_guarded)                          \
  X(DW_AT_GNU_pt_guarded)                       \
  X(DW_AT_GNU_locks_excluded)                   \
  X(DW_AT_GNU_exclusive_locks_required)         \
  X(DW_AT_GNU_shared_locks_required)            \
  X(DW_AT_GNU_odr_signature)                    \
  X(DW_AT_GNU_template_name)                    \
  X(DW_AT_GNU_call_site_value)                  \
  X(DW_AT_GNU_call_site_data_value)             \
  X(DW_AT_GNU_call_site_target)                 \
  X(DW_AT_GNU_call_site_target_clobbered)       \
  X(DW_AT_GNU_tail_call)                        \
  X(DW_AT_GNU_all_tail_call_sites)              \
  X(DW_AT_GNU_all_call_sites)                   \
  X(DW_AT_GNU_all_source_call_sites)            \
  X(DW_AT_GNU_macros)                           \
  X(DW_AT_GNU_dwo_name)                         \
  X(DW_AT_GNU_dwo_id)                           \
  X(DW_AT_GNU_ranges_base)                      \
  X(DW_AT_GNU_addr_base)                        \
  X(DW_AT_GNU_pubnames)                         \
  X(DW_AT_GNU_pubtypes)                         \
  X(DW_AT_GNU_discriminator)                    \
  X(DW_AT_ALTIUM_loclist)                       \
  X(DW_AT_SUN_template)                         \
  X(DW_AT_VMS_rtnbeg_pd_address)                \
  X(DW_AT_SUN_alignment)                        \
  X(DW_AT_SUN_vtable)                           \
  X(DW_AT_SUN_count_guarantee)                  \
  X(DW_AT_SUN_command_line)                     \
  X(DW_AT_SUN_vbase)                            \
  X(DW_AT_SUN_compile_options)                  \
  X(DW_AT_SUN_language)                         \
  X(DW_AT_SUN_browser_file)                     \
  X(DW_AT_SUN_vtable_abi)                       \
  X(DW_AT_SUN_func_offsets)                     \
  X(DW_AT_SUN_cf_kind)                          \
  X(DW_AT_SUN_vtable_index)                     \
  X(DW_AT_SUN_omp_tpriv_addr)                   \
  X(DW_AT_SUN_omp_child_func)                   \
  X(DW_AT_SUN_func_offset)                      \
  X(DW_AT_SUN_memop_type_ref)                   \
  X(DW_AT_SUN_profile_id)                       \
  X(DW_AT_SUN_memop_signature)                  \
  X(DW_AT_SUN_obj_dir)                          \
  X(DW_AT_SUN_obj_file)                         \
  X(DW_AT_SUN_original_name)                    \
  X(DW_AT_SUN_hwcprof_signature)                \
  X(DW_AT_SUN_amd64_parmdump)                   \
  X(DW_AT_SUN_part_link_name)                   \
  X(DW_AT_SUN_link_name)                        \
  X(DW_AT_SUN_pass_with_const)                  \
  X(DW_AT_SUN_return_with_const)                \
  X(DW_AT_SUN_import_by_name)                   \
  X(DW_AT_SUN_f90_pointer)                      \
  X(DW_AT_SUN_pass_by_ref)                      \
  X(DW_AT_SUN_f90_allocatable)                  \
  X(DW_AT_SUN_f90_assumed_shape_array)          \
  X(DW_AT_SUN_c_vla)                            \
  X(DW_AT_SUN_return_value_ptr)                 \
  X(DW_AT_SUN_dtor_start)                       \
  X(DW_AT_SUN_dtor_length)                      \
  X(DW_AT_SUN_dtor_state_initial)               \
  X(DW_AT_SUN_dtor_state_final)                 \
  X(DW_AT_SUN_dtor_state_deltas)                \
  X(DW_AT_SUN_import_by_lname)                  \
  X(DW_AT_SUN_f90_use_only)                     \
  X(DW_AT_SUN_namelist_spec)                    \
  X(DW_AT_SUN_is_omp_child_func)                \
  X(DW_AT_SUN_fortran_main_alias)               \
  X(DW_AT_SUN_fortran_based)                    \
  X(DW_AT_use_GNAT_descriptive_type)            \
  X(DW_AT_GNAT_descriptive_type)                \
  X(DW_AT_upc_threads_scaled)                   \
  X(DW_AT_PGI_lbase)                            \
  X(DW_AT_PGI_soffset)                          \
  X(DW_AT_PGI_lstride)                          \
  X(DW_AT_APPLE_optimized)                      \
  X(DW_AT_APPLE_flags)                          \
  X(DW_AT_APPLE_isa)                            \
  X(DW_AT_APPLE_block)                          \
  X(DW_AT_APPLE_major_runtime_vers)             \
  X(DW_AT_APPLE_runtime_class)                  \
  X(DW_AT_APPLE_omit_frame_ptr)                 \
  X(DW_AT_APPLE_closure)                        \
  X(DW_AT_APPLE_major_runtime_vers)             \
  X(DW_AT_APPLE_runtime_class)

#define DW_OPS(X)                               \
  X(DW_OP_addr, sizeof(uintptr_t))              \
  X(DW_OP_deref)                                \
  X(DW_OP_const1u, 1)                           \
  X(DW_OP_const1s, 1)                           \
  X(DW_OP_const2u, 2)                           \
  X(DW_OP_const2s, 2)                           \
  X(DW_OP_const4u, 4)                           \
  X(DW_OP_const4s, 4)                           \
  X(DW_OP_const8u, 8)                           \
  X(DW_OP_const8s, 8)                           \
  X(DW_OP_constu, -1)                           \
  X(DW_OP_consts, -1)                           \
  X(DW_OP_dup)                                  \
  X(DW_OP_drop)                                 \
  X(DW_OP_over)                                 \
  X(DW_OP_pick, 1)                              \
  X(DW_OP_swap)                                 \
  X(DW_OP_rot)                                  \
  X(DW_OP_xderef)                               \
  X(DW_OP_abs)                                  \
  X(DW_OP_and)                                  \
  X(DW_OP_div)                                  \
  X(DW_OP_minus)                                \
  X(DW_OP_mod)                                  \
  X(DW_OP_mul)                                  \
  X(DW_OP_neg)                                  \
  X(DW_OP_not)                                  \
  X(DW_OP_or)                                   \
  X(DW_OP_plus)                                 \
  X(DW_OP_plus_uconst,-1)                       \
  X(DW_OP_shl)                                  \
  X(DW_OP_shr)                                  \
  X(DW_OP_shra)                                 \
  X(DW_OP_xor)                                  \
  X(DW_OP_bra)                                  \
  X(DW_OP_eq)                                   \
  X(DW_OP_ge)                                   \
  X(DW_OP_gt)                                   \
  X(DW_OP_le)                                   \
  X(DW_OP_lt)                                   \
  X(DW_OP_ne)                                   \
  X(DW_OP_skip, 2)                              \
  X(DW_OP_lit0)                                 \
  X(DW_OP_lit1)                                 \
  X(DW_OP_lit2)                                 \
  X(DW_OP_lit3)                                 \
  X(DW_OP_lit4)                                 \
  X(DW_OP_lit5)                                 \
  X(DW_OP_lit6)                                 \
  X(DW_OP_lit7)                                 \
  X(DW_OP_lit8)                                 \
  X(DW_OP_lit9)                                 \
  X(DW_OP_lit10)                                \
  X(DW_OP_lit11)                                \
  X(DW_OP_lit12)                                \
  X(DW_OP_lit13)                                \
  X(DW_OP_lit14)                                \
  X(DW_OP_lit15)                                \
  X(DW_OP_lit16)                                \
  X(DW_OP_lit17)                                \
  X(DW_OP_lit18)                                \
  X(DW_OP_lit19)                                \
  X(DW_OP_lit20)                                \
  X(DW_OP_lit21)                                \
  X(DW_OP_lit22)                                \
  X(DW_OP_lit23)                                \
  X(DW_OP_lit24)                                \
  X(DW_OP_lit25)                                \
  X(DW_OP_lit26)                                \
  X(DW_OP_lit27)                                \
  X(DW_OP_lit28)                                \
  X(DW_OP_lit29)                                \
  X(DW_OP_lit30)                                \
  X(DW_OP_lit31)                                \
  X(DW_OP_reg0)                                 \
  X(DW_OP_reg1)                                 \
  X(DW_OP_reg2)                                 \
  X(DW_OP_reg3)                                 \
  X(DW_OP_reg4)                                 \
  X(DW_OP_reg5)                                 \
  X(DW_OP_reg6)                                 \
  X(DW_OP_reg7)                                 \
  X(DW_OP_reg8)                                 \
  X(DW_OP_reg9)                                 \
  X(DW_OP_reg10)                                \
  X(DW_OP_reg11)                                \
  X(DW_OP_reg12)                                \
  X(DW_OP_reg13)                                \
  X(DW_OP_reg14)                                \
  X(DW_OP_reg15)                                \
  X(DW_OP_reg16)                                \
  X(DW_OP_reg17)                                \
  X(DW_OP_reg18)                                \
  X(DW_OP_reg19)                                \
  X(DW_OP_reg20)                                \
  X(DW_OP_reg21)                                \
  X(DW_OP_reg22)                                \
  X(DW_OP_reg23)                                \
  X(DW_OP_reg24)                                \
  X(DW_OP_reg25)                                \
  X(DW_OP_reg26)                                \
  X(DW_OP_reg27)                                \
  X(DW_OP_reg28)                                \
  X(DW_OP_reg29)                                \
  X(DW_OP_reg30)                                \
  X(DW_OP_reg31)                                \
  X(DW_OP_breg0, -1)                            \
  X(DW_OP_breg1, -1)                            \
  X(DW_OP_breg2, -1)                            \
  X(DW_OP_breg3, -1)                            \
  X(DW_OP_breg4, -1)                            \
  X(DW_OP_breg5, -1)                            \
  X(DW_OP_breg6, -1)                            \
  X(DW_OP_breg7, -1)                            \
  X(DW_OP_breg8, -1)                            \
  X(DW_OP_breg9, -1)                            \
  X(DW_OP_breg10, -1)                           \
  X(DW_OP_breg11, -1)                           \
  X(DW_OP_breg12, -1)                           \
  X(DW_OP_breg13, -1)                           \
  X(DW_OP_breg14, -1)                           \
  X(DW_OP_breg15, -1)                           \
  X(DW_OP_breg16, -1)                           \
  X(DW_OP_breg17, -1)                           \
  X(DW_OP_breg18, -1)                           \
  X(DW_OP_breg19, -1)                           \
  X(DW_OP_breg20, -1)                           \
  X(DW_OP_breg21, -1)                           \
  X(DW_OP_breg22, -1)                           \
  X(DW_OP_breg23, -1)                           \
  X(DW_OP_breg24, -1)                           \
  X(DW_OP_breg25, -1)                           \
  X(DW_OP_breg26, -1)                           \
  X(DW_OP_breg27, -1)                           \
  X(DW_OP_breg28, -1)                           \
  X(DW_OP_breg29, -1)                           \
  X(DW_OP_breg30, -1)                           \
  X(DW_OP_breg31, -1)                           \
  X(DW_OP_regx, -1)                             \
  X(DW_OP_fbreg, -1)                            \
  X(DW_OP_bregx, -1, -1)                        \
  X(DW_OP_piece, 1)                             \
  X(DW_OP_deref_size, 1)                        \
  X(DW_OP_xderef_size, 1)                       \
  X(DW_OP_nop)                                  \
  X(DW_OP_push_object_address)                  \
  X(DW_OP_call2, 2)                             \
  X(DW_OP_call4, 4)                             \
  X(DW_OP_call_ref, -2)                         \
  X(DW_OP_form_tls_address)                     \
  X(DW_OP_call_frame_cfa)                       \
  X(DW_OP_bit_piece, -1, -1)                    \
  X(DW_OP_implicit_value, -1, -3, -4)           \
  X(DW_OP_stack_value)                          \
  X(DW_OP_implicit_pointer, -2, -1)             \
  X(DW_OP_addrx)                                \
  X(DW_OP_constx)                               \
  X(DW_OP_entry_value)                          \
  X(DW_OP_const_type)                           \
  X(DW_OP_regval_type)                          \
  X(DW_OP_deref_type)                           \
  X(DW_OP_xderef_type)                          \
  X(DW_OP_convert)                              \
  X(DW_OP_reinterpret)

namespace {
template<typename R, typename T, typename F>
HPHP::Optional<R> thingToResult(T thing, F values) {
  auto init = [] (auto v) {
    std::sort(v.begin(), v.end(), [] (const auto& a, const auto& b) {
        return a.first < b.first;
      });
    return v;
  };
  auto static things = init(values());
  auto it = std::lower_bound(things.begin(), things.end(), thing,
                             [&] (auto const& p, auto thing) {
                               return p.first < thing;
                             });
  if (it != things.end() && it->first == thing) return it->second;
  return std::nullopt;
}

template<typename T, typename F>
std::string thingToString(T thing, F values, const char* format) {
  auto s = thingToResult<const char*>(thing, values);
  if (s) return *s;
  return folly::sformat(format, thing);
}
}

#define X(x, ...) { x, #x },
std::string DwarfState::tagToString(Dwarf_Half tag) const {
  auto init = [] {
    return std::vector<std::pair<uint16_t, const char*>> { DW_TAGS(X) };
  };
  return thingToString(tag, init, "<UNKNOWN TAG({})>");
}

std::string DwarfState::attributeTypeToString(Dwarf_Half type) const {
  auto init = [] {
    return std::vector<std::pair<uint16_t, const char*>> { DW_ATTRIBUTES(X) };
  };
  return thingToString(type, init, "<UNKNOWN ATTR({})>");
}

std::string DwarfState::attributeFormToString(Dwarf_Half form) const {
  auto init = [] {
    return std::vector<std::pair<uint16_t, const char*>> { DW_FORMS(X) };
  };
  return thingToString(form, init, "<UNKNOWN FORM({})>");
}

std::string DwarfState::opToString(Dwarf_Half op) const {
  auto init = [] {
    return std::vector<std::pair<uint16_t, const char*>> { DW_OPS(X) };
  };
  return thingToString(op, init, "<UNKNOWN OP({})>");
}
#undef X

auto DwarfState::getAttributeValueExprLoc(
    Dwarf_Attribute attr) const -> std::vector<Dwarf_Loc> {

  struct Info {
    int sz1, sz2, sz3;
  };
  auto init = [] {
    #define X(x, ...) { x, {__VA_ARGS__} },
    return std::vector<std::pair<uint16_t, Info>> { DW_OPS(X) };
    #undef X
  };

  auto rawData = attr->attrValue;

  std::vector<Dwarf_Loc> ret;
  while (!rawData.empty()) {
    auto op = read<uint8_t>(rawData);
    auto info = thingToResult<Info>(op, init);
    if (!info) return std::vector<Dwarf_Loc>{};
    Dwarf_Loc loc;
    auto readRaw = [&] (uint64_t sz) {
      uint64_t v{};
      auto bytes = sz <= 8 ? sz : 8;
      auto addr = rawData.data();
      rawData.advance(bytes);
      memcpy(&v, addr, bytes);
      return v;
    };
    auto get = [&] (int sz) -> uint64_t {
      switch (sz) {
        // -3 and -4 are special handlers for implicit value
        case -3: return readRaw(loc.lr_number);
        case -4:
          if (loc.lr_number > 8) {
            auto const ret = readRaw(loc.lr_number - 8);
            if (loc.lr_number > 16) rawData.advance(loc.lr_number - 16);
            return ret;
          }
          return 0;
        case -2: return readOffset(rawData, attr->die->context->is64Bit);
        case -1: return readULEB(rawData);
        case 0: return 0;
        case 1: return read<uint8_t>(rawData);
        case 2: return read<uint16_t>(rawData);
        case 4: return read<uint32_t>(rawData);
        case 8: return read<uint64_t>(rawData);
      }
      not_reached();
    };
    loc.lr_atom = op;
    loc.lr_number = get(info->sz1);
    loc.lr_number2 = get(info->sz2);
    loc.lr_offset = get(info->sz3);
    ret.push_back(loc);
  }

  return ret;
}

////////////////////////////////////////////////////////////////////////////////

}

#endif
