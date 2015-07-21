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

#include "hphp/runtime/base/type-structure.h"

#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/util/text-util.h"

namespace HPHP {

struct String;
struct StaticString;

namespace {

/*
 * These static strings are the same as the ones in
 * hphp/compiler/type_annotation.cpp, where the typeAnnotArrays are
 * originally generated. See TypeAnnotation::getScalarArrayRep().
 */
const StaticString
  s_nullable("nullable"),
  s_name("name"),
  s_classname("classname"),
  s_kind("kind"),
  s_elem_types("elem_types"),
  s_return_type("return_type"),
  s_param_types("param_types"),
  s_generic_types("generic_types"),
  s_root_name("root_name"),
  s_access_list("access_list")
;

const std::string
  s_void("HH\\void"),
  s_int("HH\\int"),
  s_bool("HH\\bool"),
  s_float("HH\\float"),
  s_string("HH\\string"),
  s_resource("HH\\resource"),
  s_num("HH\\num"),
  s_arraykey("HH\\arraykey"),
  s_noreturn("HH\\noreturn"),
  s_mixed("HH\\mixed"),
  s_array("array")
;

std::string fullName(const ArrayData* arr);

void functionTypeName(const ArrayData* arr, std::string& name) {
  name += "(function (";

  assert(arr->exists(s_return_type));
  auto const retType = arr->get(s_return_type).getArrayData();

  assert(arr->exists(s_param_types));
  auto const params = arr->get(s_param_types).getArrayData();

  auto sz = params->getSize();
  for (auto i = 0; i < sz; i++) {
    name += fullName(params->get(i).getArrayData());
    name += ", ";
  }

  // if the function has args, erase the trailing comma
  if (sz > 1) {
    name.erase(name.size() - 2, 2);
  }
  name += "): ";

  // add funciton return type
  name += fullName(retType);
  name += ")";
}

void accessTypeName(const ArrayData* arr, std::string& name) {
  assert(arr->exists(s_root_name));
  auto const rootName = arr->get(s_root_name).getStringData();
  name += rootName->toCppString();

  assert(arr->exists(s_access_list));
  auto const accessList = arr->get(s_access_list).getArrayData();
  auto sz = accessList->getSize();
  for (auto i = 0; i < sz; i++) {
    name += "::";
    name += accessList->get(i).getStringData()->toCppString();
  }
}

// xhp names are mangled so we get them back to their original definition
// see the mangling in ScannerToken::xhpLabel
void xhpTypeName(const ArrayData* arr, std::string& name) {
  assert(arr->exists(s_classname));
  std::string clsName = arr->get(s_classname).getStringData()->toCppString();
  // remove prefix if any
  if (clsName.compare(0, sizeof("xhp_") - 1, "xhp_") == 0) {
    name += clsName.replace(0, sizeof("xhp_") - 1, ":");
  } else {
    name += clsName;
  }
  // un-mangle back
  replaceAll(name, "__", ":");
  replaceAll(name, "_", "-");
}

void tupleTypeName(const ArrayData* arr, std::string& name) {
  name += "(";
  assert(arr->exists(s_elem_types));
  auto const elems = arr->get(s_elem_types).getArrayData();
  auto sz = elems->getSize();
  for (auto i = 0; i < sz; i++) {
    name += fullName(elems->get(i).getArrayData());
    name += ", ";
  }

  // replace the trailing ", " with ">"
  name.replace(name.size() - 2, 2, ")");
}

void genericTypeName(const ArrayData* arr, std::string& name) {
  name += "<";
  assert(arr->exists(s_generic_types));
  auto const args = arr->get(s_generic_types).getArrayData();
  auto sz = args->getSize();
  for (auto i = 0; i < sz; i++) {
    name += fullName(args->get(i).getArrayData());
    name += ", ";
  }

  // replace the trailing ", " with ">"
  name.replace(name.size() - 2, 2, ">");
}

std::string fullName (const ArrayData* arr) {
  std::string name;
  if (arr->exists(s_nullable)) {
    assert(arr->get(s_nullable).getBoolean());
    name += '?';
  }

  assert(arr->exists(s_kind));

  TypeStructure::Kind kind =
    TypeStructure::Kind(arr->get(s_kind).getNumData());
  switch (kind) {
    case TypeStructure::Kind::T_void:
      name += s_void;
      break;
    case TypeStructure::Kind::T_int:
      name += s_int;
      break;
    case TypeStructure::Kind::T_bool:
      name += s_bool;
      break;
    case TypeStructure::Kind::T_float:
      name += s_float;
      break;
    case TypeStructure::Kind::T_string:
      name += s_string;
      break;
    case TypeStructure::Kind::T_resource:
      name += s_resource;
      break;
    case TypeStructure::Kind::T_num:
      name += s_num;
      break;
    case TypeStructure::Kind::T_arraykey:
      name += s_arraykey;
      break;
    case TypeStructure::Kind::T_noreturn:
      name += s_noreturn;
      break;
    case TypeStructure::Kind::T_mixed:
      name += s_mixed;
      break;
    case TypeStructure::Kind::T_tuple:
      tupleTypeName(arr, name);
      break;
    case TypeStructure::Kind::T_fun:
      functionTypeName(arr, name);
      break;
    case TypeStructure::Kind::T_array:
      name += s_array;
      if (arr->exists(s_generic_types)) {
        genericTypeName(arr, name);
      }
      break;
    case TypeStructure::Kind::T_typevar:
      assert(arr->exists(s_name));
      name += arr->get(s_name).getStringData()->toCppString();
      break;
    case TypeStructure::Kind::T_typeaccess:
      accessTypeName(arr, name);
      break;
    case TypeStructure::Kind::T_xhp:
      xhpTypeName(arr, name);
      break;
    case TypeStructure::Kind::T_unresolved:
      assert(arr->exists(s_classname));
      name += arr->get(s_classname).getStringData()->toCppString();
      if (arr->exists(s_generic_types)) {
        genericTypeName(arr, name);
      }
      break;
  }

  return name;
}

} // anonymous namespace

String TypeStructure::toString(const ArrayData* arr) {
  if (arr->empty()) {
    return String();
  }

  return String(fullName(arr));
}

} // namespace HPHP
