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

#pragma once

#include <folly/Demangle.h>
#include <folly/Format.h>
#include <folly/Memory.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>
#include <folly/portability/Unistd.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <dwarf.h>
#include <libdwarf.h>

namespace debug_parser {

////////////////////////////////////////////////////////////////////////////////

/*
 * Thrown if there's an issue while parsing dwarf debug information.
 */
struct DwarfStateException: std::runtime_error {
  using std::runtime_error::runtime_error;
};

/*
 * libdwarf uses a very low-level janky C-style interface, so provide a simple
 * wrapper class to make some of the common operations easier.
 *
 * In a few cases, libdwarf keeps internal state, which forces you walk the DIEs
 * in a hierarchial manner. For this reason, many of the operations are
 * structured as for-each style iteration.
 */

struct DwarfState {
   explicit DwarfState(std::string filename);
   DwarfState(const DwarfState&) = delete;
   DwarfState(DwarfState&&) = delete;
   ~DwarfState();

   DwarfState& operator=(const DwarfState&) = delete;
   DwarfState& operator=(DwarfState&&) = delete;

  Dwarf_Half getTag(Dwarf_Die die);
  std::string tagToString(Dwarf_Half tag);
  std::string getDIEName(Dwarf_Die die);
  Dwarf_Off getDIEOffset(Dwarf_Die die);
  Dwarf_Half getAttributeType(Dwarf_Attribute attr);
  std::string attributeTypeToString(Dwarf_Half type);
  Dwarf_Half getAttributeForm(Dwarf_Attribute attr);
  std::string getAttributeValueString(Dwarf_Attribute attr);
  Dwarf_Bool getAttributeValueFlag(Dwarf_Attribute attr);
  Dwarf_Unsigned getAttributeValueUData(Dwarf_Attribute attr);
  Dwarf_Signed getAttributeValueSData(Dwarf_Attribute attr);
  Dwarf_Addr getAttributeValueAddr(Dwarf_Attribute attr);
  Dwarf_Off getAttributeValueRef(Dwarf_Attribute attr);
  Dwarf_Sig8 getAttributeValueSig8(Dwarf_Attribute attr);
  std::vector<Dwarf_Loc> getAttributeValueExprLoc(Dwarf_Attribute attr);

  template <typename F> void forEachChild(Dwarf_Die die, F&& f);
  template <typename F> void forEachAttribute(Dwarf_Die die, F&& f);
  template <typename F> void forEachCompilationUnit(F&& f);
  template <typename F> auto onDIEAtOffset(Dwarf_Off offset, F&& f) ->
    decltype(f(std::declval<Dwarf_Die>()));

  int fd;
  Dwarf_Debug dwarf;
  std::string filename;
};

/*
 * Iterate over all children of this DIE, calling the given callable for
 * each. Iteration is stopped early if any of the calls return false.
 */
template <typename F> void DwarfState::forEachChild(Dwarf_Die die, F&& f) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Die prev = nullptr;
  SCOPE_EXIT {
    if (prev) dwarf_dealloc(dwarf, prev, DW_DLA_DIE);
  };

  if (die) {
    // prev is null here, and dwarf_child returns the first child if given a
    // previous DIE of null.
    auto result = dwarf_child(die, &prev, &error);
    if (result == DW_DLV_ERROR) {
      throw DwarfStateException{
        folly::sformat(
          "Unable to read child DIE: {}",
          dwarf_errmsg(error)
        )
      };
    } else if (result == DW_DLV_NO_ENTRY || !f(prev)) {
      return;
    }
  }

  while (true) {
    Dwarf_Die next = nullptr;
    SCOPE_EXIT {
      if (next) dwarf_dealloc(dwarf, next, DW_DLA_DIE);
    };

    auto result = dwarf_siblingof_b(
      dwarf, prev, true,
      &next, &error
    );
    if (result == DW_DLV_ERROR) {
      throw DwarfStateException{
        folly::sformat(
          "Unable to read sibling DIE: {}",
          dwarf_errmsg(error)
        )
      };
    } else if (result == DW_DLV_NO_ENTRY || !f(next)) {
      break;
    }

    // Swap prev and next. This will ensure the previous DIE gets freed (because
    // of the above SCOPE_EXIT).
    std::swap(prev, next);
  }
}

/*
 * Iterate over all attributes of the given DIE, calling the given callable for
 * each. Iteration is stopped early if any of the calls return false.
 */
template <typename F> void DwarfState::forEachAttribute(Dwarf_Die die, F&& f) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Attribute* attributes;
  Dwarf_Signed attribute_count;
  auto result = dwarf_attrlist(die, &attributes, &attribute_count, &error);
  if (result == DW_DLV_ERROR) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to read DIE attribute-list: {}",
        dwarf_errmsg(error)
      )
    };
  } else if (result == DW_DLV_NO_ENTRY) {
    return;
  }

  SCOPE_EXIT {
    for (Dwarf_Unsigned i = 0; i < attribute_count; ++i) {
      dwarf_dealloc(dwarf, attributes[i], DW_DLA_ATTR);
    }
    dwarf_dealloc(dwarf, attributes, DW_DLA_LIST);
  };

  for (Dwarf_Unsigned i = 0; i < attribute_count; ++i) {
    if (!f(attributes[i])) break;
  }
}

/*
 * Iterate over all the compilation-units in the file, calling the given
 * callable for each.
 */
template <typename F> void DwarfState::forEachCompilationUnit(F&& f) {
  if (!dwarf) return;

  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  while (true) {
    Dwarf_Unsigned next_cu_header = 0;
    auto result = dwarf_next_cu_header_d(
      dwarf, true, nullptr, nullptr,
      nullptr, nullptr, nullptr, nullptr,
      nullptr, nullptr, &next_cu_header,
      nullptr, &error
    );

    if (result == DW_DLV_NO_ENTRY) {
      break;
    } else if (result == DW_DLV_ERROR) {
      throw DwarfStateException{
        folly::sformat(
          "Unable to read next compilation-unit header: {}",
          dwarf_errmsg(error)
        )
      };
    }

    forEachChild(
      nullptr,
      [&](Dwarf_Die die){
        if (getTag(die) != DW_TAG_compile_unit) {
          throw DwarfStateException{
            folly::sformat(
              "First tag in compilation-unit is not DW_TAG_compile_unit ({})",
              tagToString(getTag(die))
            )
          };
        }
        f(die);
        return true;
      }
    );
  }
}

/*
 * Load the DIE at the given offset, and call the given callable on it,
 * returning whatever the callable returns.
 */
template <typename F> auto DwarfState::onDIEAtOffset(Dwarf_Off offset, F&& f) ->
  decltype(f(std::declval<Dwarf_Die>())) {

  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Die die = nullptr;
  auto result = dwarf_offdie_b(
    dwarf, offset, true,
    &die, &error
  );
  if (result != DW_DLV_OK) {
    throw DwarfStateException{
      folly::sformat(
        "Unable to read DIE at offset {}: {}",
        offset,
        dwarf_errmsg(error)
      )
    };
  }

  SCOPE_EXIT { dwarf_dealloc(dwarf, die, DW_DLA_DIE); };
  return f(die);
}

////////////////////////////////////////////////////////////////////////////////

}
