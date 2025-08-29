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

#include "hphp/runtime/base/tv-val.h"

#include "hphp/runtime/vm/generics-info.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/base/types.h"

#include "hphp/util/hash-map.h"

namespace HPHP {

struct ActRec;
struct ArrayData;
struct Class;
struct Func;
struct StringData;

///////////////////////////////////////////////////////////////////////////////

// Returns either newly created or already cached static array
ArrayData* addToTypeReifiedGenericsTable(const StringData* mangledName,
                                     ArrayData* tsList);

///////////////////////////////////////////////////////////////////////////////

// Returns the value on the property that holds reified generics
// If the cls does not have any reified generics, then returns nullptr
ArrayData* getClsReifiedGenericsProp(Class* cls, ObjectData* obj);

// Returns a GenericsInfo that contains a vector of TypeParamInfo
// which specifies whether each generic param is reified, soft or warn
// only. Format of the input is a list of integers where the first one
// is the count of reified generics and the following numbers are their
// indices and whether they are soft or not and whether to error or warn
// interleaved
GenericsInfo
extractSizeAndPosFromReifiedAttribute(
  const ArrayData* arr,
  const folly::Range<const PackedStringPtr*>& typeParamNames
);

// Extract and return only the size from the attribute array
size_t extractSizeFromReifiedAttribute(const ArrayData* arr);

// Raises a runtime error if the location of reified generics of f/c does not
// match the location of reified_generics
void checkFunReifiedGenericMismatch(
  const Func* f,
  const ArrayData* reified_generics
);
void checkClassReifiedGenericMismatch(
  const Class* c,
  const ArrayData* reified_generics
);

uint16_t getGenericsBitmap(const ArrayData* generics);
uint16_t getGenericsBitmap(const Func*);

// Returns whether all the generics in the given GenericsInfo are denoted
// as soft
bool areAllGenericsSoft(const GenericsInfo& info);
bool areAllGenericsSoft(const ArrayData* generics);

// Raises warning for parameter at index i for function/class name
void raise_warning_for_soft_reified(size_t i, bool fun, const StringData *name);

// Raises warning if any reified generics on the class are not soft,
// otherwise throws an error
void checkClassReifiedGenericsSoft(const Class* cls);


/*
 * Attempt to invoke 86reifiedinit on the given object.
 * Will setReifiedGenerics iff the class has a reified parent.
 * The function throws if the class has reified generics and
 * any reified generics on the class are not soft.
 * If no reified generics are passed in, AND all
 * reified generics on the class are soft, then raises an error.
*/
void tryClassReifiedInit(Class* cls, ArrayData* generics, ObjectData* obj);
///////////////////////////////////////////////////////////////////////////////

}
