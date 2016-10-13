/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP {

struct Func;
struct Class;
struct StringData;
struct ObjectData;

enum class CallType {
  ClsMethod,
  ObjMethod,
  CtorMethod,
};

enum class LookupResult {
  MethodFoundWithThis,
  MethodFoundNoThis,
  MagicCallFound,
  MagicCallStaticFound,
  MethodNotFound,
};

const Func* lookupMethodCtx(const Class* cls,
                            const StringData* methodName,
                            const Class* pctx,
                            CallType lookupType,
                            bool raise = false);
LookupResult lookupObjMethod(const Func*& f,
                             const Class* cls,
                             const StringData* methodName,
                             const Class* ctx,
                             bool raise = false);
LookupResult lookupClsMethod(const Func*& f,
                             const Class* cls,
                             const StringData* methodName,
                             ObjectData* this_,
                             const Class* ctx,
                             bool raise = false);
LookupResult lookupCtorMethod(const Func*& f,
                              const Class* cls,
                              const Class* ctx,
                              bool raise = false);

}
