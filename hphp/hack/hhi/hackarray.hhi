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

<<__Sealed(dict::class, keyset::class, vec::class)>>
abstract class AnyArray<+Tk as arraykey, +Tv> implements KeyedContainer<Tk, Tv>, \XHPChild {}

abstract final class dict<+Tk as arraykey, +Tv> extends AnyArray<Tk, Tv> {}
abstract final class keyset<+T as arraykey> extends AnyArray<T, T> {}
abstract final class vec<+T> extends AnyArray<int, T> {}

} // namespace HH
