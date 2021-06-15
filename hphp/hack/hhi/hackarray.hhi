<?hh // partial
/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH {

/**
 * The parent class for all array types (containers that are values).
 * This currently includes both Hack Arrays (vec, dict, keyset) and Legacy
 * Arrays (varray, darray).
 */
<<__Sealed(dict::class, keyset::class, vec::class), __SupportDynamicType>>
abstract class AnyArray<+Tk as arraykey, +Tv> implements KeyedContainer<Tk, Tv>, \XHPChild {

}

<<__SupportDynamicType>>
abstract final class dict<+Tk as arraykey, +Tv> extends AnyArray<Tk, Tv> {}
<<__SupportDynamicType>>
abstract final class keyset<+T as arraykey> extends AnyArray<T, T> {}
<<__SupportDynamicType>>
abstract final class vec<+T> extends AnyArray<int, T> {}

} // namespace HH
