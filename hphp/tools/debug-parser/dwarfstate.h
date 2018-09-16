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
#include <folly/container/F14Map.h>
#include <folly/portability/Unistd.h>

#include <stdexcept>
#include <string>
#include <vector>
#include <functional>

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

struct GlobalOff {
  GlobalOff(Dwarf_Off off, bool isInfo) : m_value{off * 2 + (isInfo ? 1 : 0)} {
    assert(offset() == off);
  }

  static GlobalOff fromRaw(uint64_t raw) {
    return GlobalOff{raw};
  }
  Dwarf_Off offset() const { return m_value >> 1; }
  bool isInfo() const { return m_value & 1; }
  uint64_t raw() const { return m_value; }

  friend GlobalOff operator+(GlobalOff a, size_t b) {
    return {a.offset() + b, a.isInfo()};
  }
  friend bool operator<(GlobalOff a, GlobalOff b) {
    // we want all the debug_types to sort before all the debug_infos
    if ((a.m_value ^ b.m_value) & 1) {
      return (a.m_value & 1);
    }
    return a.m_value < b.m_value;
  }
  friend bool operator==(GlobalOff a, GlobalOff b) {
    return a.raw() == b.raw();
  }
  friend bool operator>(GlobalOff a, GlobalOff b) { return b < a; }
  friend bool operator<=(GlobalOff a, GlobalOff b) { return !(a > b); }
  friend bool operator>=(GlobalOff a, GlobalOff b) { return !(a < b); }
  friend bool operator!=(GlobalOff a, GlobalOff b) { return !(a == b); }
  struct Hash {
    size_t hash(GlobalOff a) const {
      return std::hash<uint64_t>{}(a.raw());
    }
    bool equal(GlobalOff a, GlobalOff b) const {
      return a == b;
    }
    size_t operator()(GlobalOff a) const { return hash(a); }
  };
private:
  explicit GlobalOff(uint64_t raw) : m_value{raw} {}
  uint64_t m_value;
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
  using Sig8Map = folly::F14FastMap<uint64_t, GlobalOff>;

  explicit DwarfState(std::string filename, const Sig8Map* sig8);
  DwarfState(const DwarfState&) = delete;
  DwarfState(DwarfState&&) = delete;
  ~DwarfState();

  DwarfState& operator=(const DwarfState&) = delete;
  DwarfState& operator=(DwarfState&&) = delete;

  static uint64_t Sig8AsKey(Dwarf_Sig8 sig);
  Dwarf_Half getTag(Dwarf_Die die);
  std::string tagToString(Dwarf_Half tag);
  std::string getDIEName(Dwarf_Die die);
  GlobalOff getDIEOffset(Dwarf_Die die);
  Dwarf_Half getAttributeType(Dwarf_Attribute attr);
  std::string attributeTypeToString(Dwarf_Half type);
  Dwarf_Half getAttributeForm(Dwarf_Attribute attr);
  std::string getAttributeValueString(Dwarf_Attribute attr);
  Dwarf_Bool getAttributeValueFlag(Dwarf_Attribute attr);
  Dwarf_Unsigned getAttributeValueUData(Dwarf_Attribute attr);
  Dwarf_Signed getAttributeValueSData(Dwarf_Attribute attr);
  Dwarf_Addr getAttributeValueAddr(Dwarf_Attribute attr);
  GlobalOff getAttributeValueRef(Dwarf_Die die, Dwarf_Attribute attr);
  GlobalOff getAttributeValueRef(Dwarf_Attribute attr);
  Dwarf_Sig8 getAttributeValueSig8(Dwarf_Attribute attr);
  std::vector<Dwarf_Loc> getAttributeValueExprLoc(Dwarf_Attribute attr);
  std::vector<Dwarf_Ranges> getRanges(Dwarf_Off offset);

  template <typename F> void forEachChild(Dwarf_Die die, F&& f);
  template <typename F> void forEachAttribute(Dwarf_Die die, F&& f);
  template <typename F> void forEachCompilationUnit(F&& f);
  template <typename F> void forEachTopLevelUnit(F&& f, bool isInfo);
  template <typename F> auto onDIEAtOffset(GlobalOff offset, F&& f) ->
    decltype(f(std::declval<Dwarf_Die>()));
  template <typename F> auto onDIEAtIncreasingOffset(GlobalOff offset, F&& f) ->
    decltype(f(std::declval<Dwarf_Die>()));

  int fd;
  Dwarf_Debug dwarf;
  std::string filename;
  const Sig8Map* sig8_map;

  Dwarf_Off cur_info_offset{0};
  Dwarf_Off next_info_offset{0};
  Dwarf_Off cur_type_offset{0};
  Dwarf_Off next_type_offset{0};
private:
  template <typename F>
  static auto caller(const F& f,
                     Dwarf_Die die,
                     Dwarf_Sig8 sig,
                     GlobalOff type_offset) ->
    decltype(f(die, sig, type_offset));

  template <typename F>
  static auto caller(const F& f,
                     Dwarf_Die die,
                     Dwarf_Sig8 sig,
                     GlobalOff type_offset) ->
    decltype(f(die), true);
  template <typename F> void forEachChildHelper(Dwarf_Die die,
                                                bool isInit, F&& f);
};

inline uint64_t DwarfState::Sig8AsKey(Dwarf_Sig8 sig) {
  union {
    uint64_t val;
    Dwarf_Sig8 sig;
  } v;
  v.sig = sig;
  return v.val;
}

/*
 * Iterate over all children of this DIE, calling the given callable for
 * each. Iteration is stopped early if any of the calls return false.
 */
template <typename F>
void DwarfState::forEachChildHelper(Dwarf_Die die, bool isInit, F&& f) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Die prev = nullptr;
  SCOPE_EXIT {
    if (prev) dwarf_dealloc(dwarf, prev, DW_DLA_DIE);
  };

  assert(!die || isInit == dwarf_get_die_infotypes_flag(die));

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
      dwarf, prev, isInit,
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

template <typename F>
void DwarfState::forEachChild(Dwarf_Die die, F&& f) {
  assert(die);
  forEachChildHelper(die, dwarf_get_die_infotypes_flag(die), std::move(f));
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

template <typename F>
auto DwarfState::caller(const F& f,
                        Dwarf_Die die,
                        Dwarf_Sig8 sig,
                        GlobalOff type_offset) ->
  decltype(f(die, sig, type_offset)) {
  return f(die, sig, type_offset);
}

template <typename F>
auto DwarfState::caller(const F& f,
                        Dwarf_Die die,
                        Dwarf_Sig8,
                        GlobalOff) ->
  decltype(f(die), true) {
  f(die);
  return true;
}

/*
 * Iterate over all the compilation-units in the file, calling the given
 * callable for each.
 */
template <typename F> void DwarfState::forEachTopLevelUnit(F&& f, bool isInit) {
  if (!dwarf) return;

  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Unsigned cur_cu_header = 0;
  while (true) {
    Dwarf_Unsigned next_cu_header = 0;
    Dwarf_Unsigned type_offset;
    Dwarf_Sig8 sig;
    auto result = dwarf_next_cu_header_d(
      dwarf, isInit, nullptr, nullptr,
      nullptr, nullptr, nullptr, nullptr,
      &sig, &type_offset, &next_cu_header,
      nullptr, &error
    );

    if (result == DW_DLV_NO_ENTRY) {
      break;
    }

    if (result == DW_DLV_ERROR) {
      throw DwarfStateException{
        folly::sformat(
          "Unable to read next compilation-unit header: {}",
          dwarf_errmsg(error)
        )
      };
    }

    forEachChildHelper(
      nullptr,
      isInit,
      [&](Dwarf_Die die){
        switch (getTag(die)) {
          case DW_TAG_compile_unit:
          case DW_TAG_type_unit:
            return caller(f, die, sig, {cur_cu_header + type_offset, isInit});
          default:
            throw DwarfStateException{
              folly::sformat(
                  "First tag in compilation-unit is not "
                  "DW_TAG_compile_unit ({})",
                  tagToString(getTag(die))
              )
            };
        }
      }
    );
    cur_cu_header = next_cu_header;
  }
}

template <typename F> void DwarfState::forEachCompilationUnit(F&& f) {
  forEachTopLevelUnit(f, true);
}

/*
 * Load the DIE at the given offset, and call the given callable on it,
 * returning whatever the callable returns.
 */
template <typename F> auto DwarfState::onDIEAtOffset(GlobalOff offset, F&& f) ->
  decltype(f(std::declval<Dwarf_Die>())) {

  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Die die = nullptr;
  auto result = dwarf_offdie_b(
      dwarf, offset.offset(), offset.isInfo(),
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

template <typename F>
auto DwarfState::onDIEAtIncreasingOffset(GlobalOff offset, F&& f) ->
  decltype(f(std::declval<Dwarf_Die>())) {

  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  auto isInfo = offset.isInfo();
  auto off = offset.offset();

  Dwarf_Unsigned cur_header = isInfo ? cur_info_offset : cur_type_offset;
  assert(off >= cur_header);
  Dwarf_Unsigned next_header = isInfo ? next_info_offset : next_type_offset;
  while (off >= next_header) {
    cur_header = next_header;

    auto result = dwarf_next_cu_header_d(
        dwarf, isInfo, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, &next_header,
        nullptr, &error
    );

    if (result == DW_DLV_NO_ENTRY) {
      cur_header = next_header = 0;
      break;
    }

    if (result == DW_DLV_ERROR) {
      throw DwarfStateException{
        folly::sformat(
            "Unable to read next compilation-unit header: {}",
            dwarf_errmsg(error)
        )
      };
    }
  }

  if (isInfo) {
    cur_info_offset = cur_header;
    next_info_offset = next_header;
  } else {
    cur_type_offset = cur_header;
    next_type_offset = next_header;
  }

  Dwarf_Die die = nullptr;
  auto result = dwarf_offdie_b(
      dwarf, offset.offset(), offset.isInfo(),
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

////////////////////////////////////////////////////////////////////////////////

namespace folly {
template<> class FormatValue<debug_parser::GlobalOff> {
 public:
  explicit FormatValue(debug_parser::GlobalOff val) : m_val(val) {}

  template<typename Callback>
  void format(FormatArg& arg, Callback& cb) const {
    format_value::formatString(folly::sformat("{}:{}",
                                              m_val.offset(),
                                              m_val.isInfo() ? 1 : 0),
                               arg, cb);
  }

 private:
  debug_parser::GlobalOff m_val;
};
}

namespace std {
template<>
struct hash<debug_parser::GlobalOff> : debug_parser::GlobalOff::Hash {};
}

////////////////////////////////////////////////////////////////////////////////
