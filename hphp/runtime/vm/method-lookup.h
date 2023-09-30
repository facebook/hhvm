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

#include "hphp/util/either.h"
#include "hphp/runtime/vm/class.h"

namespace HPHP {

struct Func;
struct Class;
struct Unit;
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
  MethodNotFound,
};

enum class MethodLookupErrorOptions {
  None                       = 0,
  RaiseOnNotFound            = (1u << 0),
  NoErrorOnModule            = (1u << 1),
};


const Func* lookupMethodCtx(const Class* cls,
                            const StringData* methodName,
                            const MemberLookupContext& callCtx,
                            CallType lookupType,
                            MethodLookupErrorOptions raise);

LookupResult lookupObjMethod(const Func*& f,
                             const Class* cls,
                             const StringData* methodName,
                             const MemberLookupContext& callCtx,
                             MethodLookupErrorOptions raise);

/*
 * This routine attempts to find the Func* that will be called for an object
 * of a given target Class (which may be an interface) and a function name,
 * when called from ctxFunc.
 *
 * If exactClass is true, the class we are targeting is assumed to be
 * exactly `cls', and the returned Func* is definitely the one called.
 *
 * If exactClass is false, the class we are targeting may be a subclass of
 * `cls` or a class implementing `cls`, and the returned Func* may be
 * overridden in a subclass.
 *
 * The returned Func* may be used in a request-insensitive way, i.e. it is
 * suitable for burning into the TC as a pointer.
 *
 * It's the caller's responsibility to ensure that the Class* is usable -
 * is AttrUnique, an instance of the ctx or guarded in some way.
 *
 * Returns (NotFound, nullptr) if we can't determine the Func*.
 */
struct ImmutableObjMethodLookup {
  enum class Type {
    NotFound,   // unable to determine suitable Func*
    Func,       // the called func is returned
    Class,      // the called func may override the returned base class func
    Interface,  // the called func implements the returned interface func
  };

  Type type;
  const Func* func;
};

ImmutableObjMethodLookup
lookupImmutableObjMethod(const Class* cls,
                         const StringData* name,
                         const MemberLookupContext& callCtx,
                         bool exactClass);

LookupResult lookupClsMethod(const Func*& f,
                             const Class* cls,
                             const StringData* methodName,
                             ObjectData* this_,
                             const MemberLookupContext& callCtx,
                             MethodLookupErrorOptions raise);

/*
 * This routine attempts to find the Func* that will be statically called for
 * a given target Class and function name, when called from ctxFunc.
 *
 * If exactClass is true, the class we are targeting is assumed to be
 * exactly `cls', and the returned Func* is definitely the one called.
 *
 * If exactClass is false, the class we are targeting may be a subclass of
 * `cls`, and the returned Func* may be overridden in a subclass.
 *
 * The returned Func* may be used in a request-insensitive way, i.e. it is
 * suitable for burning into the TC as a pointer.
 *
 * It's the caller's responsibility to ensure that the Class* is usable -
 * is AttrUnique, an instance of the ctx or guarded in some way.
 *
 * Returns nullptr if we can't determine the Func*.
 */
const Func* lookupImmutableClsMethod(const Class* cls,
                                     const StringData* name,
                                     const MemberLookupContext& callCtx,
                                     bool exactClass);

LookupResult lookupCtorMethod(const Func*& f,
                              const Class* cls,
                              const MemberLookupContext& callCtx,
                              MethodLookupErrorOptions raise);

/*
 * If possible find the constructor for cls that would be run from the
 * context ctx if a new instance of cls were created there.  If the
 * constructor is inaccessible from the given context this function
 * will return nullptr. It is the caller's responsibility to ensure
 * that cls is the right Class* (ie its AttrUnique or bound to the
 * ctx, or otherwise guaranteed by guards).
 */
const Func* lookupImmutableCtor(const Class* cls,
                                const MemberLookupContext& callCtx);

/*
 * Find a function which always uniquely maps to the given name in the context
 * of the given unit. A function so returned can be used directly in the TC as
 * it will not change.
 */
Func* lookupImmutableFunc(const StringData* name);

}
