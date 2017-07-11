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

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace debug_parser {

////////////////////////////////////////////////////////////////////////////////

DwarfState::DwarfState(std::string filename)
  : fd{-1}
  , dwarf{nullptr}
  , filename{std::move(filename)}
{
  fd = open(this->filename.c_str(), O_RDONLY);
  if (fd < 0) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to open file '{}': {}",
        this->filename,
        folly::errnoStr(errno)
      )
    };
  }

  // Ignore relocations for executable files as processing them takes
  // a lot more time and memory but adds no value.
  struct stat buf;
  fstat(fd, &buf);
  if (buf.st_mode & S_IXUSR) dwarf_set_reloc_application(0);

  Dwarf_Error error = nullptr;
  if (dwarf_init(
        fd,
        DW_DLC_READ,
        nullptr,
        nullptr,
        &dwarf,
        &error
      ) == DW_DLV_ERROR) {
    SCOPE_EXIT { if (error) free(error); };
    SCOPE_EXIT { close(fd); };
    throw DwarfStateException{
      folly::sformat(
        "Unable to init libdwarf on file '{}': {}",
        this->filename,
        dwarf_errmsg(error)
      )
    };
  }
}

DwarfState::~DwarfState() {
  Dwarf_Error error = nullptr;
  if (dwarf) dwarf_finish(dwarf, &error);
  close(fd);
}

Dwarf_Half DwarfState::getTag(Dwarf_Die die) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Half tag = 0;
  if (dwarf_tag(die, &tag, &error) != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to read DIE tag: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return tag;
}

std::string DwarfState::tagToString(Dwarf_Half tag) {
  const char* tag_name = nullptr;
  auto result = dwarf_get_TAG_name(tag, &tag_name);
  if (result == DW_DLV_NO_ENTRY) {
    return folly::sformat("<UNKNOWN({})>", tag);
  }
  return tag_name;
}

std::string DwarfState::getDIEName(Dwarf_Die die) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  char* raw_name = nullptr;
  auto result = dwarf_diename(die, &raw_name, &error);
  if (result == DW_DLV_ERROR) {
    throw DwarfStateException{
      folly::sformat(
          "Unable to read DIE name: {}",
          dwarf_errmsg(error)
      )
    };
  } else if (result == DW_DLV_NO_ENTRY) {
    return "";
  } else {
    SCOPE_EXIT { dwarf_dealloc(dwarf, raw_name, DW_DLA_STRING); };
    return raw_name;
  }
}

Dwarf_Off DwarfState::getDIEOffset(Dwarf_Die die) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Off offset = 0;
  auto result = dwarf_dieoffset(die, &offset, &error);
  if (result != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
          "Unable to read DIE offset: {}",
          dwarf_errmsg(error)
      )
    };
  }

  return offset;
}

Dwarf_Half DwarfState::getAttributeType(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Half what_attr = 0;
  auto result = dwarf_whatattr(attr, &what_attr, &error);
  if (result != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to obtain attribute type: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return what_attr;
}

std::string DwarfState::attributeTypeToString(Dwarf_Half type) {
  const char* attr_name;
  auto result = dwarf_get_AT_name(type, &attr_name);
  if (result == DW_DLV_NO_ENTRY) {
    return folly::sformat("<UNKNOWN({})>", type);
  }
  return attr_name;
}

Dwarf_Half DwarfState::getAttributeForm(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Half form = 0;
  auto result = dwarf_whatform(attr, &form, &error);
  if (result != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to obtain attribute form: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return form;
}

std::string DwarfState::getAttributeValueString(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  char* str = nullptr;
  auto result = dwarf_formstring(attr, &str, &error);
  if (result != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to obtain attribute value string: {}",
        dwarf_errmsg(error)
      )
    };
  }

  SCOPE_EXIT { dwarf_dealloc(dwarf, str, DW_DLA_STRING); };
  return str;
}

Dwarf_Bool DwarfState::getAttributeValueFlag(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  if (getAttributeForm(attr) == DW_FORM_flag_present) {
    return true;
  }

  Dwarf_Bool value;
  auto result = dwarf_formflag(attr, &value, &error);
  if (result != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to obtain attribute value flag: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return value;
}

Dwarf_Unsigned DwarfState::getAttributeValueUData(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Unsigned value;
  auto result = dwarf_formudata(attr, &value, &error);
  if (result != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to obtain attribute value unsigned data: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return value;
}

Dwarf_Signed DwarfState::getAttributeValueSData(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Signed value;
  auto result = dwarf_formsdata(attr, &value, &error);
  if (result != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to obtain attribute value signed data: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return value;
}

Dwarf_Addr DwarfState::getAttributeValueAddr(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Addr value;
  auto result = dwarf_formaddr(attr, &value, &error);
  if (result != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to obtain attribute value address: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return value;
}

Dwarf_Off DwarfState::getAttributeValueRef(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Off value;
  auto result = dwarf_global_formref(attr, &value, &error);
  if (result != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to obtain attribute value ref: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return value;
}

Dwarf_Sig8 DwarfState::getAttributeValueSig8(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Sig8 value;
  auto result = dwarf_formsig8(attr, &value, &error);
  if (result != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to obtain attribute value sig8: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return value;
}

std::vector<Dwarf_Loc>
DwarfState::getAttributeValueExprLoc(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Ptr raw_ptr;
  Dwarf_Unsigned raw_len;
  auto result = dwarf_formexprloc(attr, &raw_len, &raw_ptr, &error);
  if (result != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to obtain attribute value exprloc: {}",
        dwarf_errmsg(error)
      )
    };
  }

  Dwarf_Locdesc* locations = nullptr;
  Dwarf_Signed locations_count = 0;
  SCOPE_EXIT {
    if (locations) {
      for (Dwarf_Signed i = 0; i < locations_count; ++i) {
        dwarf_dealloc(dwarf, locations[i].ld_s, DW_DLA_LOC_BLOCK);
      }
      dwarf_dealloc(dwarf, locations, DW_DLA_LOCDESC);
    }
  };

  result = dwarf_loclist_from_expr(
    dwarf,
    raw_ptr,
    raw_len,
    &locations,
    &locations_count,
    &error
  );
  if (result != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to turn exprloc into location list: {}",
        dwarf_errmsg(error)
      )
    };
  }

  if (locations_count != 1) {
    throw DwarfStateException{
      "Obtained more than one location list from exprloc"
    };
  }

  return std::vector<Dwarf_Loc>{
    locations->ld_s,
    locations->ld_s + locations->ld_cents
  };
}

////////////////////////////////////////////////////////////////////////////////

}

#endif
