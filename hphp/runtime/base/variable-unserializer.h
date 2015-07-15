/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VARIABLE_UNSERIALIZER_H_
#define incl_HPHP_VARIABLE_UNSERIALIZER_H_

#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/types.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

enum class UnserializeMode {
  Value = 0,
  Key = 1,
  ColValue = 2,
  ColKey = 3,
};

struct VariableUnserializer {

  /*
   * Supported unserialization formats.
   */
  enum class Type {
    Serialize,
    APCSerialize,
    DebuggerSerialize
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
    const Array& classWhitelist = null_array);

  /*
   * Main API; unserialize the buffer and return as a Variant.
   */
  Variant unserialize();

  /*
   * Read the appropriate data type from buffer.
   */
  int64_t readInt();
  double readDouble();
  char readChar();
  void read(char* buf, unsigned n);

  /*
   * Read a character and throw if it differs from expected.
   */
  void expectChar(char expected);
  void throwUnexpected(char expected, char got);

  /*
   * Accessors.
   */
  Type type() const;
  bool allowUnknownSerializableClass() const;
  const char* head() const;
  char peek() const;
  char peekBack() const;
  bool endOfBuffer() const;

  /*
   * True if clsName is allowed to be unserialized.
   */
  bool isWhitelistedClass(const String& clsName) const;

  /*
   * Set the beginning and end of internal buffer.
   */
  void set(const char* buf, const char* end);

  /*
   * Push v onto the vector of refs for future reference.
   */
  void add(Variant* v, UnserializeMode mode);

  /*
   * Used by the 'r' encoding to get a reference.
   */
  Variant* getByVal(int id);

  /*
   * Used by the 'R' encoding to get a reference.
   */
  Variant* getByRef(int id);

  /*
   * Store properties/array elements that get overwritten incase they are
   * referenced later during unserialization
   */
  void putInOverwrittenList(const Variant& v);

private:
  /*
   * Hold references to previously-unserialized data, along with bits telling
   * whether it is legal to reference them later.
   */
  struct RefInfo {
    explicit RefInfo(Variant* v);
    static RefInfo makeNonRefable(Variant* v);
    Variant* var() const;
    bool canBeReferenced() const;
  private:
    uintptr_t m_data;
  };

  Array m_overwrittenList;

  void check() const;

  Type m_type;
  const char* m_buf;
  const char* m_end;
  req::vector<RefInfo> m_refs;
  bool m_unknownSerializable;
  const Array& m_classWhiteList;    // classes allowed to be unserialized
};

///////////////////////////////////////////////////////////////////////////////

void unserializeVariant(Variant&, VariableUnserializer *unserializer,
                        UnserializeMode mode = UnserializeMode::Value);

}

#include "hphp/runtime/base/variable-unserializer-inl.h"

#endif // incl_HPHP_VARIABLE_UNSERIALIZER_H_
