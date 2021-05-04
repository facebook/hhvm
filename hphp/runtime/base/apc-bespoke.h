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

#include <atomic>

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////

struct ArrayData;
struct APCTypedValue;

/*
 * The `ad` field of this return value will always be non-nullptr. If bespokes
 * are disabled, it'll match the input; otherwise, it may be a bespoke-layout
 * version of the input, based on layout decisions.
 *
 * The `tv` field will be non-nullptr if and only if we made an APCBespoke -
 * that is, a wrapper around APCTypedValue that profiles lookups on this APC
 * stored value. In this case, the allocation is managed by us, and the caller
 * should init `tv` with an APCKind of StaticBespoke / UncountedBespoke.
 */
struct APCBespoke {
  ArrayData* ad = nullptr;
  APCTypedValue* tv = nullptr;
};

// Call this method on creating a new APCTypedValue wrapping `ad`.
APCBespoke initAPCBespoke(ArrayData* ad);

// Call this method to wrap of an APCBespoke-profiled APCTypedValue on read.
ArrayData* readAPCBespoke(const APCTypedValue* tv);

// Call this method to destroy an APCBespoke-profiled APCTypedValue.
void freeAPCBespoke(APCTypedValue* tv);

//////////////////////////////////////////////////////////////////////////////

}
