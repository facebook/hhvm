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

#include "hphp/util/assertions.h"

#include <folly/Demangle.h>
#include <folly/Format.h>
#include <folly/Memory.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>
#include <folly/container/F14Map.h>
#include <folly/portability/Unistd.h>
#include <folly/experimental/symbolizer/Elf.h>

#include <atomic>
#include <functional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <dwarf.h>

namespace debug_parser {

////////////////////////////////////////////////////////////////////////////////

/*
 * Thrown if there's an issue while parsing dwarf debug information.
 */
struct DwarfStateException: std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct GlobalOff {
  GlobalOff(uint64_t off, bool isInfo) : m_value{off * 2 + (isInfo ? 1 : 0)} {
    assert(offset() == off);
  }
  GlobalOff(int64_t off, bool isInfo) :
      GlobalOff{ static_cast<uint64_t>(off), isInfo } {}

  static GlobalOff fromRaw(uint64_t raw) {
    return GlobalOff{raw};
  }
  uint64_t offset() const { return m_value >> 1; }
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

struct AbbrevMap {
  void build(folly::StringPiece debug_abbrev);

  static uint64_t readOne(folly::StringPiece& section,
                          uint64_t &tag, bool &hasChildren,
                          folly::StringPiece& attrs);

  std::vector<uint64_t> abbrev_vec;
  // map from offset to abbrev_vec index
  folly::F14FastMap<uint64_t, uint64_t> abbrev_map;
};

struct DwarfState {
  using Sig8Map = folly::F14FastMap<uint64_t, GlobalOff>;

  explicit DwarfState(std::string filename);
  DwarfState(const DwarfState&) = delete;
  DwarfState(DwarfState&&) = delete;
  ~DwarfState();

  DwarfState& operator=(const DwarfState&) = delete;
  DwarfState& operator=(DwarfState&&) = delete;

  /*
   * A top level chunk in the .debug_info or .debug_types section.
   *
   * Contains a compilation-unit, or type-unit, and all its children.
   *
   * section + offset + size points at the start of the next context.
   */
  struct Context {
    const char* section;
    uint64_t offset;
    uint64_t size;
    bool is64Bit;
    bool isInfo;
    uint8_t version;
    uint8_t addrSize;
    uint64_t abbrevOffset;
    uint64_t typeSignature;
    uint64_t typeOffset;
    uint64_t firstDie;
  };

  struct Die {
    const Context* context;
    // offset within context->section
    uint64_t offset;
    uint64_t code;
    uint64_t tag;
    // convenience copy from the context.
    bool is64Bit;
    bool hasChildren;
    // offset from start to first attribute
    uint8_t attrOffset;
    // if we know where the next sibling is (eg via DW_AT_sibling), and
    // it fits in uint32_t, the delta we need to add to offset to get
    // there; otherwise zero.
    uint32_t siblingDelta;
    // if we know where the next die is, and it fits in uint32_t, this
    // is the delta we need to add to offset to get there; otherwise zero.
    // if there are no children, this will be the same as sibling.
    uint32_t nextDieDelta;
    folly::StringPiece attributes;
  };

  struct AttributeSpec {
    uint64_t name{};
    uint64_t form{};

    explicit operator bool() const {
      return name || form;
    }
  };

  struct Attribute : AttributeSpec {
    Attribute(AttributeSpec spec, Die* die, folly::StringPiece sp) :
        AttributeSpec{spec}, die{die}, attrValue{sp} {}
    Die* die;
    folly::StringPiece attrValue;
  };

  using Dwarf_Half = uint16_t;
  struct Dwarf_Loc {
    Dwarf_Half lr_atom;
    uint64_t lr_number;
    uint64_t lr_number2;
    uint64_t lr_offset;
  };
  struct Dwarf_Ranges {
    static auto constexpr kSelection = uintptr_t(-1);

    uintptr_t dwr_addr1;
    uintptr_t dwr_addr2;
  };

  Context getContextAtOffset(GlobalOff off) const;
  Die getDieAtOffset(const Context* context, GlobalOff off) const;
  Die getNextSibling(Die* die) const;
  Dwarf_Half getTag(Die* die) const;
  std::string tagToString(Dwarf_Half tag) const;
  std::string getDIEName(Die* die) const;
  GlobalOff getDIEOffset(Die* die) const;
  Dwarf_Half getAttributeType(Attribute* attr) const;
  std::string attributeTypeToString(Dwarf_Half type) const;
  Dwarf_Half getAttributeForm(Attribute* attr) const;
  std::string attributeFormToString(Dwarf_Half form) const;
  std::string opToString(Dwarf_Half form) const;
  std::string getAttributeValueString(Attribute* attr) const;
  folly::StringPiece getAttributeValueStringPiece(Attribute* attr) const;
  bool getAttributeValueFlag(Attribute* attr) const;
  uint64_t getAttributeValueUData(Attribute* attr) const;
  int64_t getAttributeValueSData(Attribute* attr) const;
  uintptr_t getAttributeValueAddr(Attribute* attr) const;
  GlobalOff getAttributeValueRef(Attribute* attr) const;
  uint64_t getAttributeValueSig8(Attribute* attr) const;
  std::vector<Dwarf_Loc> getAttributeValueExprLoc(Attribute* attr) const;
  std::vector<Dwarf_Ranges> getRanges(Attribute* attr) const;

  // Get a string from the .debug_str section
  folly::StringPiece getStringFromStringSection(uint64_t offset) const;

  template <typename F> void forEachContext(F&& f, bool isInfo) const;
  template <typename F> void forEachContextParallel(F&& f, bool isInfo,
                                                    int num_threads) const;
  template <typename F> void forEachChild(Die* die, F&& f) const;
  template <typename F> void forEachAttribute(Die* die, F&& f) const;
  template <typename F> void forEachCompilationUnit(F&& f) const;
  template <typename F> void forEachTopLevelUnit(F&& f, bool isInfo) const;
  template <typename F> void forEachTopLevelUnitParallel(F&& f, bool isInfo,
                                                         int num_threads) const;
  template <typename F> auto onDIEAtOffset(GlobalOff offset, F&& f) const ->
    decltype(f(std::declval<Die*>()));
  template <typename F> auto onDIEAtContextOffset(
      GlobalOff contextOff, F&& f) const ->
    decltype(f(std::declval<Die*>()));

  static AttributeSpec readAttributeSpec(folly::StringPiece&);
  static Attribute readAttribute(Die* die, AttributeSpec spec,
                                 folly::StringPiece& sp);

  // Read (bitwise) one object of type T
  template <class T>
  static typename std::enable_if<std::is_pod<T>::value, T>::type read(
      folly::StringPiece& sp) {
    assert(sp.size() >= sizeof(T));
    T x;
    memcpy(&x, sp.data(), sizeof(T));
    sp.advance(sizeof(T));
    return x;
  }
private:
  AbbrevMap abbrevMap;
  Sig8Map sig8_map;
  std::vector<uint64_t> cuContextOffsets;
  std::vector<uint64_t> tuContextOffsets;

  folly::symbolizer::ElfFile elf;
  folly::StringPiece debug_info;
  folly::StringPiece debug_types;
  folly::StringPiece debug_abbrev;
  folly::StringPiece debug_str;
  folly::StringPiece debug_ranges;

  // Read a value of "section offset" type, which may be 4 or 8 bytes
  static uint64_t readOffset(folly::StringPiece& sp, bool is64Bit) {
    return is64Bit ? read<uint64_t>(sp) : read<uint32_t>(sp);
  }

  static void updateDelta(uint32_t& delta, uint64_t value) {
    if (value != static_cast<uint32_t>(value)) {
      assertx(!delta);
      return;
    }
    if (!delta) {
      delta = value;
      return;
    }
    assertx(delta == value);
  }

  void init();
};

using Dwarf_Die = DwarfState::Die*;
using Dwarf_Context = DwarfState::Context*;
using Dwarf_Attribute = DwarfState::Attribute*;

/*
 * Iterate over all children of this DIE, calling the given callable for
 * each. Iteration is stopped early if any of the calls return false.
 */
template <typename F>
void DwarfState::forEachChild(Dwarf_Die die, F&& f) const {
  if (!die->hasChildren) return;

  if (!die->nextDieDelta) {
    forEachAttribute(die, [] (Dwarf_Attribute) { return true; });
    assert(die->nextDieDelta);
  }

  auto sibling = getDieAtOffset(
      die->context, { die->offset + die->nextDieDelta, die->context->isInfo }
  );
  while (sibling.code) {
    if (!f(&sibling)) return;
    sibling = getNextSibling(&sibling);
  }

  // sibling is a dummy die whose offset is to the code 0 marking the
  // end of the children. Need to add one to get the offset of the
  // next die
  updateDelta(die->siblingDelta, sibling.offset + 1 - die->offset);
}

/*
 * Iterate over all attributes of the given DIE, calling the given callable for
 * each. Iteration is stopped early if any of the calls return false.
 */
template <typename F>
void DwarfState::forEachAttribute(Dwarf_Die die, F&& f) const {
  auto attrs = die->attributes;
  auto values = folly::StringPiece {
    die->context->section + die->offset + die->attrOffset,
    die->context->section + die->context->offset + die->context->size
  };
  while (auto const aspec = readAttributeSpec(attrs)) {
    auto attr = readAttribute(die, aspec, values);
    if (!die->siblingDelta && attr.name == DW_AT_sibling) {
      updateDelta(die->siblingDelta,
                  getAttributeValueRef(&attr).offset() - die->offset);
      if (!die->hasChildren) {
        assert(!die->nextDieDelta || die->nextDieDelta == die->siblingDelta);
        die->nextDieDelta = die->siblingDelta;
      }
    }
    if (!f(&attr)) return;
  }

  updateDelta(die->nextDieDelta,
              values.data() - die->context->section - die->offset);
  if (!die->hasChildren) {
    assertx(!die->siblingDelta || die->siblingDelta == die->nextDieDelta);
    die->siblingDelta = die->nextDieDelta;
  }
}

/*
 * Iterate over all the contexts in the file, calling the given
 * callable for each.
 */
template <typename F>
void DwarfState::forEachContext(F&& f, bool isInfo) const {
  auto const section = isInfo ? debug_info : debug_types;
  auto sp = section;
  while (!sp.empty()) {
    auto context = getContextAtOffset(
        { sp.data() - section.data(), isInfo }
    );
    sp.advance(context.size);
    f(&context);
  }
}

/*
 * Iterate over all the contexts in the file, calling the given
 * callable for each in parellel. The function f must be thread safe.
 */
template <typename F>
void DwarfState::forEachContextParallel(F&& f, bool isInfo,
                                        int num_threads) const {
  if (num_threads <= 1) return forEachContext(f, isInfo);

  std::vector<GlobalOff> offsets;

  forEachContext([&] (Dwarf_Context context) {
    offsets.push_back({ context->offset, isInfo });
  }, isInfo);

  std::atomic<size_t> index{0};

  std::vector<std::thread> workers;
  for (auto worker = size_t{0}; worker < num_threads; ++worker) {
    workers.push_back(std::thread([&] {
      while (true) {
        const size_t kChunkSize = 50;
        auto start = index.fetch_add(kChunkSize, std::memory_order_relaxed);
        if (start >= offsets.size()) break;
        for (auto i = start;
             i < start + kChunkSize && i < offsets.size();
             ++i) {
          auto context = getContextAtOffset(offsets[i]);
          f(&context);
        }
      }
    }));
  }
  for (auto& t : workers) t.join();
}

/*
 * Iterate over all the compilation-units in the file, calling the given
 * callable for each.
 */
template <typename F>
void DwarfState::forEachTopLevelUnitParallel(F&& f, bool isInfo,
                                             int num_threads) const {
  forEachContextParallel(
    [&] (Dwarf_Context context) {
      auto die = getDieAtOffset(context, { context->firstDie, isInfo });
      f(&die);
    },
    isInfo,
    num_threads
  );
}

template <typename F>
void DwarfState::forEachTopLevelUnit(F&& f, bool isInfo) const {
  forEachTopLevelUnitParallel(f, isInfo, 1);
}

template <typename F> void DwarfState::forEachCompilationUnit(F&& f) const {
  forEachTopLevelUnit(std::forward<F>(f), true);
}

/*
 * Load the DIE at the given offset, and call the given callable on it,
 * returning whatever the callable returns.
 */
template <typename F> auto DwarfState::onDIEAtOffset(GlobalOff offset,
                                                     F&& f) const ->
  decltype(f(std::declval<Dwarf_Die>())) {

  auto const& contextOffsets = offset.isInfo() ?
    cuContextOffsets : tuContextOffsets;
  auto it = std::upper_bound(
      contextOffsets.begin(), contextOffsets.end(), offset.offset());
  assertx(it != contextOffsets.begin());
  auto const contextOffset = *--it;
  auto const context = getContextAtOffset({ contextOffset, offset.isInfo() });

  auto die = getDieAtOffset(&context, offset);
  return f(&die);
}

template <typename F>
auto DwarfState::onDIEAtContextOffset(GlobalOff offset, F&& f) const ->
  decltype(f(std::declval<Dwarf_Die>())) {

  auto context = getContextAtOffset(offset);
  auto die = getDieAtOffset(&context, { context.firstDie, context.isInfo });
  return f(&die);
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
