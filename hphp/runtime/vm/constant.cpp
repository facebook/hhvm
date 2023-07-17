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

#include "hphp/runtime/vm/constant.h"

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/tv-refcount.h"

#include "hphp/runtime/vm/coeffects.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/native.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbc);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_stdin("STDIN");
const StaticString s_stdout("STDOUT");
const StaticString s_stderr("STDERR");

//////////////////////////////////////////////////////////////////////

}

void Constant::prettyPrint(std::ostream& out) const {
  out << "Constant " << name->data();
  if (type(val) == KindOfUninit) {
    out << " = " << "<non-scalar>";
  } else {
    std::string ss;
    staticStreamer(&val, ss);
    out << " = " << ss;
  }
  out << std::endl;
}

const StringData* Constant::nameFromFuncName(const StringData* func_name) {
  const int prefix_len = sizeof("86cinit_") - 1;
  if (func_name->size() <= prefix_len) {
    return nullptr;
  }
  auto slice = func_name->slice();
  auto ns_pos = slice.rfind('\\');
  folly::StringPiece ns_slice = "";
  if (ns_pos >= 0) {
    ns_slice = slice.subpiece(0, ns_pos + 1);
    slice = slice.subpiece(ns_pos + 1, slice.size() - ns_pos - 1);
  }
  if (!slice.startsWith("86cinit_")) {
    return nullptr;
  }
  slice = slice.subpiece(8, slice.size() - prefix_len);
  return makeStaticString(folly::sformat("{}{}", ns_slice, slice));
}

const StringData* Constant::funcNameFromName(const StringData* name) {
  auto slice = name->slice();
  auto ns_pos = slice.rfind('\\');
  if (ns_pos < 0) {
    return makeStaticString(folly::sformat("86cinit_{}", slice));
  }
  auto ns_slice = slice.subpiece(0, ns_pos + 1);
  slice = slice.subpiece(ns_pos + 1, slice.size() - ns_pos - 1);
  return makeStaticString(folly::sformat("{}86cinit_{}", ns_slice, slice));
}

TypedValue Constant::lookup(const StringData* cnsName) {
  auto const handle = lookupCnsHandle(cnsName);

  if (LIKELY(rds::isHandleBound(handle) &&
             rds::isHandleInit(handle))) {
    auto const& tv = rds::handleToRef<TypedValue, rds::Mode::NonLocal>(handle);

    if (LIKELY(type(tv) != KindOfUninit)) {
      assertx(tvIsPlausible(tv));
      tvIncRefGen(tv);
      return tv;
    }

    Variant v = Constant::get(cnsName);
    const TypedValue tvRet = v.detach();
    assertx(tvIsPlausible(tvRet));
    assertx(tvAsCVarRef(&tvRet).isAllowedAsConstantValue() ==
            Variant::AllowedAsConstantValue::Allowed);

    if (rds::isNormalHandle(handle) && type(tvRet) != KindOfResource) {
      tvIncRefGen(tvRet);
      rds::handleToRef<TypedValue, rds::Mode::Normal>(handle) = tvRet;
    }
    return tvRet;
  }
  return make_tv<KindOfUninit>();
}

const TypedValue* Constant::lookupPersistent(const StringData* cnsName) {
  auto const handle = lookupCnsHandle(cnsName);
  if (!rds::isHandleBound(handle) || !rds::isPersistentHandle(handle)) {
    return nullptr;
  }
  auto const ret = rds::handleToPtr<TypedValue, rds::Mode::Persistent>(handle);
  assertx(tvIsPlausible(*ret));
  return ret;
}

TypedValue Constant::load(const StringData* cnsName) {
  auto const tv = lookup(cnsName);
  if (LIKELY(type(tv) != KindOfUninit)) return tv;

  if (needsNSNormalization(cnsName)) {
    return load(normalizeNS(cnsName));
  }

  if (!AutoloadHandler::s_instance->autoloadConstant(
        const_cast<StringData*>(cnsName))) {
    return make_tv<KindOfUninit>();
  }
  return lookup(cnsName);
}

Variant Constant::get(const StringData* name) {
  const StringData* func_name = Constant::funcNameFromName(name);
  Func* func = Func::lookup(func_name);
  assertx(
    func &&
    "The function should have been autoloaded when we loaded the constant");
  return Variant::attach(
    g_context->invokeFuncFew(func, nullptr, 0, nullptr,
                             RuntimeCoeffects::fixme(), false, false)
  );
}

void Constant::def(const Constant* constant) {
  auto const cnsName = constant->name;
  FTRACE(3, "  Defining def {}\n", cnsName->data());
  auto const cnsVal = constant->val;

  if (constant->attrs & Attr::AttrPersistent) {
    DEBUG_ONLY auto res = bindPersistentCns(cnsName, cnsVal);
    assertx(res);
    return;
  }

  auto const ch = makeCnsHandle(cnsName);
  assertx(rds::isHandleBound(ch));

  if (rds::isHandleInit(ch)) {
    raise_error(Strings::CONSTANT_ALREADY_DEFINED, cnsName->data());
  }

  assertx(cnsVal.m_type == KindOfUninit ||
          tvAsCVarRef(&cnsVal).isAllowedAsConstantValue() ==
          Variant::AllowedAsConstantValue::Allowed);

  assertx(rds::isNormalHandle(ch));
  auto cns = rds::handleToPtr<TypedValue, rds::Mode::NonLocal>(ch);
  tvDup(cnsVal, *cns);
  rds::initHandle(ch);
}

}
