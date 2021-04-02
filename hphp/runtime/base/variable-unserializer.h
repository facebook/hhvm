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

#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/util/compact-tagged-ptrs.h"

namespace HPHP {
struct StringBuffer;

///////////////////////////////////////////////////////////////////////////////

enum class UnserializeMode {
  Value = 0,
  Key = 1,
  ColKey = 2,
};

struct InvalidAllowedClassesException : Exception {
};

struct VariableUnserializer {

  /*
   * Supported unserialization formats.
   */
  enum class Type {
    Serialize,
    Internal,
    APCSerialize,
    DebuggerSerialize,
    Last
  };

  /*
   * Construct an unserializer, with an optional whitelist of classes to
   * allow. In the whitelist, empty_array means "allow no classes", while
   * null_array means allow any classes.  We default to null_array since
   * serialization is not limited inside the VM.
   */
  VariableUnserializer(
    const char* str,
    size_t len,
    Type type,
    bool allowUnknownSerializableClass = false,
    const Array& options = null_array);

  /*
   * Optimize for output that is expected to be immortal and immutable.
   */
  void setReadOnly() { m_readOnly = true; }

  /*
   * Main API; unserialize the buffer and return as a Variant.
   */
  Variant unserialize();

  void reserialize(StringBuffer& buf);

  const char* head() const;

  /*
   * Set the beginning and end of internal buffer.
   */
  void set(const char* buf, const char* end);

  void setUnitFilename(const StringData* name) {
    assertx(name->isStatic());
    assertx(m_type == Type::Internal);
    m_unitFilename = name;
  }

 private:
  /*
   * Used to object property values that are overwritten by later entries.
   * For array element values, later values for the key override earlier ones.
   */
  Array m_overwrittenList = Array::CreateVec();

  bool readOnly() const { return m_readOnly; }

  /*
   * Read the appropriate data type from buffer.
   */
  int64_t readInt();
  double readDouble();
  char readChar();

  // Return a StringPiece of up to n characters pointing into m_buf
  folly::StringPiece readStr(unsigned n);

  /*
   * Read a character and throw if it differs from expected.
   */
  void expectChar(char expected);

  /*
   * Attempt to consume a serialized string with content matching str.
   * Return false and rewind stream on non-standard format or content mismatch.
   */
  bool matchString(folly::StringPiece str);

  /*
   * Accessors.
   */
  Type type() const;
  bool allowUnknownSerializableClass() const;
  const char* begin() const;
  const char* end() const;
  char peek() const;
  char peekBack() const;
  bool endOfBuffer() const;

  /*
   * True if clsName is allowed to be unserialized.
   */
  bool whitelistCheck(const String& clsName) const;

  /*
   * Push v onto the vector of refs for future reference.
   */
  void add(tv_lval v, UnserializeMode mode);

  /*
   * Preallocate memory for an expected number of values to be added
   * (excluding those with mode UnserializeMode::Key).
   */
  void reserveForAdd(size_t count);

  /*
   * Used by the 'r' encoding to re-use previously deserialized values.
   */
  TypedValue getByVal(int id);

  /*
   * Register an object that needs its __wakeup() method called after
   * unserialization of the top-level value is complete.
   */
  void addSleepingObject(const Object&);

private:
  void check() const;
  void checkElemTermination() const;

  Type m_type;
  bool m_readOnly;
  const char* m_buf;
  const char* m_end;
  req::vector<tv_rval> m_refs;
  bool m_unknownSerializable;
  const Array& m_options; // e.g. classes allowed to be unserialized
  req::vector<Object> m_sleepingObjects;
  const char* const m_begin;
  bool m_forceDArrays;
  bool m_markLegacyArrays;
  /* unitFilename should be set when we are deserializing
   * an adata from the repo--it is needed to *re*construct the
   * correct provenance tag */
  const StringData* m_unitFilename{nullptr};

  void unserializeVariant(tv_lval self,
                          UnserializeMode mode = UnserializeMode::Value);
  Array unserializeArray();
  void unserializeProvenanceTag();
  Array unserializeDict();
  Array unserializeVec();
  Array unserializeKeyset();
  Array unserializeVArray();
  Array unserializeDArray();
  folly::StringPiece unserializeStringPiece(char delimiter0 = '"',
                                            char delimiter1 = '"');
  String unserializeString(char delimiter0 = '"', char delimiter1 = '"');
  void unserializeCollection(ObjectData* obj, int64_t sz, char type);
  void unserializeVector(ObjectData*, int64_t sz,
                         char type);
  void unserializeMap(ObjectData*, int64_t sz, char type);
  void unserializeSet(ObjectData*, int64_t sz, char type);
  void unserializePair(ObjectData*, int64_t sz, char type);
  void unserializePropertyValue(tv_lval v, int remainingProps);
  bool tryUnserializeStrIntMap(struct BaseMap* map, int64_t sz);
  void unserializeProp(ObjectData* obj, const String& key, Class* ctx,
                       const String& realKey, int nProp);
  void unserializeRemainingProps(Object& obj, int remainingProps,
                                 Variant& serializedNativeData,
                                 bool& hasSerializedNativeData);
};

}

