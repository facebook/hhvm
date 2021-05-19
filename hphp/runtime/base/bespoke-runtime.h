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

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/mixed-array.h"

#include <folly/container/F14Map.h>
#include <folly/SharedMutex.h>

#include <string>

namespace HPHP {

namespace bespoke {
struct StructLayout;
struct LoggingProfile;
}

namespace jit {
struct RuntimeStructSerde;
}

//////////////////////////////////////////////////////////////////////////////

/*
 * A RuntimeStruct corresponds to a single array creation site in C++ code.
 * Note that the code which registers RuntimeStructs may decide how to split
 * its uses into separate array creation sites (for example, a deserialization
 * routine may declare one array creation site per schema of data it
 * deserializes). In order to create a RuntimeStruct, an identifier for the
 * given C++ array creation site must be provided. This identifier is a string
 * which must be consistent across invocations of HHVM: it will be serialized
 * and used to associate the array creation site with this RuntimeStruct upon
 * deserialization.
 *
 * In addition to the identifier, a list of fields the array creation site will
 * set must also be provided. These fields are represented as a vector of pairs
 * {index, key}. The index is a caller-chosen integer associated with the given
 * key. The key is the string key to be used in the array. The index will be
 * used in the StructDictInit initializer to set the corresponding field,
 * bypassing hashtable lookup. The indices may be chosen as the caller desires,
 * but they should be small integers: the translation table between
 * RuntimeStruct indices and slot indices in the selected layout consumes space
 * proportional to the largest RuntimeStruct index.
 *
 * During layout selection, each RuntimeStruct is assigned either a
 * StructLayout or a vanilla layout using the same mechanism as Hack array
 * creation sites.
 */
struct RuntimeStruct {
  using FieldIndexVector = std::vector<std::pair<size_t, String>>;

  /*
   * Creates or retrieves a RuntimeStruct corresponding to the supplied
   * stableIdentifier. If it is a new identifier, a RuntimeStruct with the
   * supplied field set is created. If the identifier has been seen previously,
   * the field set is validated against the previous version.
   *
   * In the case bespokes are disabled, it returns nullptr, which is valid to
   * supply to StructDictInit.
   */
  static RuntimeStruct* registerRuntimeStruct(
      const String& stableIdentifier, const FieldIndexVector& fields);
  static void eachRuntimeStruct(std::function<void(RuntimeStruct*)>);

  static RuntimeStruct* findById(const StringData* stableIdentifier);

  const StringData* toString() const;
  const StringData* getStableIdentifier() const;
  size_t stableHash() const;

  void applyLayout(const bespoke::StructLayout* layout);

  /*
   * A LoggingProfile cache to avoid having to access the LoggingProfile map
   * every time we produce a LoggingArray for this creation site.
   */
  std::atomic<bespoke::LoggingProfile*> m_profile;

private:
  using FieldKeys = std::vector<StringData*>;

  RuntimeStruct(
      const StringData* stableIdentifier,
      const FieldIndexVector& fields,
      size_t fieldsLength);
  RuntimeStruct(
      const StringData* stableIdentifier, FieldKeys&& fields);
  static RuntimeStruct* deserialize(
      const StringData* stableIdentifier, FieldKeys&& fields);

  void validateFieldsMatch(const FieldIndexVector& fields) const;

  // NOTE: RuntimeStruct is followed in memory by an array of m_fields.size()
  // Slot values, because we read the slots in hot code if we select a struct
  // layout for this array source.
  //
  // These five methods are the only ones that deal with the layout logic.

  static RuntimeStruct* allocate(size_t fieldsLength);

  LowStringPtr getKey(size_t index) const;
  Slot getSlot(size_t index) const;

  void setKey(size_t index, StringData* key);
  void setSlot(size_t index, Slot slot);

  // Private fields, along with extra array.

  const StringData* m_stableIdentifier;
  CompactVector<LowStringPtr> m_fields;
  std::atomic<const bespoke::StructLayout*> m_assignedLayout;

  friend struct StructDictInit;
  friend struct jit::RuntimeStructSerde;
};

//////////////////////////////////////////////////////////////////////////////

/*
 * The initializer for a runtime source that may be specialized to a
 * RuntimeStruct. Callers much only set fields which have been provided to the
 * associated RuntimeStruct.
 *
 * During profiling, this initializer will use sampling to generate
 * LoggingArrays, akin to a Hack array creation site. After layout selection,
 * it will create arrays of the chosen layout: either a specific StructDict, or
 * vanilla.
 *
 * The constructor accepts a RuntimeStruct to attach to a given runtime source,
 * and an initial capacity, which is used only when a vanilla layout is
 * selected.
 */
struct StructDictInit {
  StructDictInit(RuntimeStruct* structHandle, size_t n);
  ~StructDictInit();
  void set(size_t idx, StringData* key, TypedValue value);
  void set(size_t idx, const String& key, TypedValue value);
  void set(size_t idx, const String& key, const Variant& value);
  void set(int64_t key, const Variant& value);
  void set(int64_t key, TypedValue value);
  void setIntishCast(size_t idx, const String& key, const Variant& value);

  Variant toVariant();
  Array toArray();

private:
  ArrayData* m_arr;
  RuntimeStruct* m_struct;
  uint32_t m_escalateCapacity;
  bool m_vanilla;
};

}
