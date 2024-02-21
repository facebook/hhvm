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

#include <folly/json/dynamic.h>

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/func.h"

#include "hphp/util/low-ptr.h"

namespace HPHP {

struct Class;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct ProfDataSerializer;
struct ProfDataDeserializer;

struct MethProfile {
  using RawType = LowPtr<Class>::storage_type;

  enum class Tag : uint8_t {
    UniqueClass = 0,
    UniqueMeth = 1,
    BaseMeth = 2,
    InterfaceMeth = 3,
    Invalid = 4
  };

  /////////////////////////////////////////////////////////////////////////////

  MethProfile() : m_curMeth(nullptr), m_curClass(nullptr) {}

  MethProfile(const MethProfile& other)
    : m_curMeth(other.m_curMeth)
    , m_curClass(other.m_curClass)
  {}

  std::string toString() const;
  folly::dynamic toDynamic() const;

  /*
   * Obtain the profiled Class* or method Func*.
   */
  const Class* uniqueClass() const {
    return curTag() == Tag::UniqueClass ? rawClass() : nullptr;
  }
  const Func* uniqueMeth() const {
    return (curTag() == Tag::UniqueMeth ||
            curTag() == Tag::UniqueClass) ? rawMeth() : nullptr;
  }
  const Func* baseMeth() const {
    return curTag() == Tag::BaseMeth ? rawMeth() : nullptr;
  }
  const Func* interfaceMeth() const {
    return curTag() == Tag::InterfaceMeth ? rawMeth() : nullptr;
  }

  /*
   * Register a call to the callee frame `ar'.
   *
   * If `cls' is not provided (when it's not known statically), we peek in `ar'
   * for the class context.
   */
  void reportMeth(const Class* cls, const Func* meth, const Func* callerFunc);

  /*
   * Aggregate two MethProfiles.
   */
  static void reduce(MethProfile& a, const MethProfile& b);

  void serialize(ProfDataSerializer&) const;
  void deserialize(ProfDataDeserializer&);

  /////////////////////////////////////////////////////////////////////////////

private:
  /*
   * m_curMeth munging.
   */
  static Tag toTag(uintptr_t val) {
    return static_cast<Tag>(val & 7);
  }
  static const Func* fromValue(uintptr_t value) {
    return (Func*)(value & uintptr_t(-8));
  }

  /*
   * Raw value accessors.
   */
  const Class* rawClass() const { return m_curClass; }
  const Func* rawMeth() const { return fromValue(methValue()); }
  Tag curTag() const { return toTag(methValue()); }
  const uintptr_t methValue() const { return uintptr_t(m_curMeth.get()); }

  void setMeth(const Func* meth, Tag tag) {
    auto encoded_meth = (Func*)(uintptr_t(meth) | static_cast<uintptr_t>(tag));
    m_curMeth = encoded_meth;
  }

private:
  AtomicLowPtr<const Func,
               std::memory_order_acquire, std::memory_order_release> m_curMeth;
  AtomicLowPtr<const Class,
               std::memory_order_acquire, std::memory_order_release> m_curClass;
};

///////////////////////////////////////////////////////////////////////////////

}}

