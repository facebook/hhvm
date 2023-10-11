<?hh
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
  abstract class AnyArray<
    <<__NoAutoBound>> +Tk as arraykey,
    <<__NoAutoBound>> +Tv,
  > implements KeyedContainer<Tk, Tv>, \XHPChild {

  }

  /**
   * A dict is an ordered, key-value data structure.
   *
   * `dict` is a value type, so any mutation produces a new value.
   */
  <<__SupportDynamicType>>
  abstract final class dict<
    <<__NoAutoBound>> +Tk as arraykey,
    <<__NoAutoBound>> +Tv,
  > extends AnyArray<Tk, Tv> {}

  /**
   * A `keyset` is an ordered data structure without duplicates. `keyset`s can only contain `arraykey` values.
   *
   * `keyset` is a value type, so any mutation produces a new value.
   */
  <<__SupportDynamicType>>
  abstract final class keyset<<<__NoAutoBound>> +T as arraykey>
    extends AnyArray<T, T> {}

  /**
   * A `vec` is an ordered, iterable data structure. The name is short for 'vector'.
   *
   * `vec` is a value type, so any mutation produces a new value.
   */
  <<__SupportDynamicType>>
  abstract final class vec<<<__NoAutoBound>> +T> extends AnyArray<int, T> {}

} // namespace HH
