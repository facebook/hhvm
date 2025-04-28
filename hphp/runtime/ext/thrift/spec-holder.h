/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <memory>

#include "hphp/runtime/ext/thrift/transport.h"
#include "hphp/runtime/ext/thrift/util.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/util/fixed-vector.h"

namespace HPHP::thrift {

struct FieldSpec {
  int16_t fieldNum{};
  TType type{};
  StringData* name{};
  bool isUnion{};
  TType ktype{}, vtype{};
  const FieldSpec& key() const {
    if (!key_) thrift_error("no key type in spec", ERR_INVALID_DATA);
    return *key_;
  }
  const FieldSpec& val() const {
    if (!val_) thrift_error("no val type in spec", ERR_INVALID_DATA);
    return *val_;
  }
  StringData* format{};
  StrNR className() const {
    if (!className_) thrift_error("no class type in spec", ERR_INVALID_DATA);
    return StrNR(className_);
  }
  Class* adapter{};
  bool isWrapped{}; // true if the field has field_wrapper annotation
  bool isTerse{}; // true if the field is marked as terse
  bool isTypeWrapped{}; // true if a field has wrapped type
  bool noTypeCheck{}; // If this field doesn't need type checking
                      // (conservatively).
  static FieldSpec compile(const Array& fieldSpec, bool topLevel);

private:
  std::unique_ptr<FieldSpec> key_, val_;
  StringData* className_{};
};

struct StructSpec {
  FixedVector<FieldSpec> fields;
  const Func* withDefaultValuesFunc;
  Optional<const Func*> clearTerseFieldsFunc;

  Object newObject(Class* cls) const;
  void clearTerseFields(Class* cls, const Object& obj) const;
};

// Provides safe access to specifications.
struct SpecHolder {
  // The returned reference is valid at least while this SpecHolder is alive.
  const StructSpec& getSpec(const Class* cls);

private:
  // Non-static spec, or empty if source spec is static.
  StructSpec m_tempSpec;
};

const FieldSpec* getFieldSlow(const StructSpec& spec, int16_t fieldNum);

}
