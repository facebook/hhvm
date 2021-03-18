<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib;

/** Wrapper class for getting object (byref) semantics for a value type.
 *
 * This is especially useful for mutating values outside of a lambda's scope.
 *
 * In general, it's preferable to refactor to use return values or `inout`
 * parameters instead of using this class - however, a `Ref` of a Hack array
 * is generally preferable to a Hack collection - e.g. prefer `Ref<vec<T>>`
 * over `Vector<T>`.
 *
 * `C\reduce()` and `C\reduce_with_key()` can also be used in some situations
 * to avoid this class.
 */
final class Ref<T> {
  public function __construct(public T $value)[rx_shallow] {}
}
