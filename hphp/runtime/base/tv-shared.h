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

#include "hphp/runtime/base/data-walker.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////

/*
 * Global parameters for ConvertTvToShared and friends. Only one for now.
 * If `seen` is provided, we'll use it to de-dupe new shared arrays.
 */
struct MakeSharedEnv {
  using ArrayMap = req::fast_map<ArrayData*, ArrayData*, pointer_hash<ArrayData>>;
  using StringSet = req::fast_set<StringData*, string_data_hash, string_data_same>;

  ArrayMap* seenArrays;
  StringSet* seenStrings;
};

/*
 * Wrappers around uncounted_malloc, etc. that update APC stats.
 */
void* AllocShared(size_t bytes);
void FreeShared(void* ptr);
void FreeShared(void* ptr, size_t bytes);

/*
 * Converts TypedValue `tv` to a shared form, so that it can be shared
 * across requests. The result is either a primitive, a static value, or
 * a shared value. Does not dec-ref the input.
 *
 * For refcounted and shared inputs, this operation produces a net increase
 * of one "shared refcount". For refcounted inputs, it creates a new value
 * with shared refcount 1, and for shared, it does an sharedIncRef().
 * (Primitives and statics are not refcounted in any way.)
 *
 * "hasApcTv" is a request to leave space for an APCTypedValue just before the
 * new shared array. We may not honor this request. For instance, if we can
 * reuse an existing persistent array, or use a static empty one, we'll do so.
 */
void ConvertTvToShared(tv_lval in, const MakeSharedEnv& env);
ArrayData* MakeSharedArray(ArrayData* in, const MakeSharedEnv& env,
                              bool hasApcTv = false);
StringData* MakeSharedString(StringData* in, const MakeSharedEnv& env);

/*
 * The analogue of decRefAndRelease for a shared value. These helpers all
 * handle both static and shared values correctly. It's safe to call them
 * on any key or value of a shared array.
 */
void DecRefShared(TypedValue tv);
void DecRefSharedArray(ArrayData* ad);
void DecRefSharedString(StringData* sd);

//////////////////////////////////////////////////////////////////////////////

}
