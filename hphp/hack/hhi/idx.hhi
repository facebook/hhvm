<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH {

  // NB: the typechecker relies on the exact format of this signature and rewrites
  // parts of it in place during each call. Changes to the signature need to be
  // done in tandem with changes to the ocaml code that munges it.
  //
  // Calls to `idx` are rewritten by the typechecker depending on their arity. It
  // can have two signatures:
  //
  // idx<Tk, Tv>(?KeyedContainer<Tk, ?Tv> $collection, ?Tk $index): ?Tv
  // idx<Tk, Tv>(?KeyedContainer<Tk, Tv> $collection, ?Tk $index, Tv $default): Tv
  /**
   * Index into the given KeyedContainer using the provided key.
   *
   * If the key doesn't exist, the key is `null`, or the collection is `null`,
   * return the provided default value instead, or `null` if no default value was
   * provided. If the key is `null`, the default value will be returned even if
   * `null` is a valid key in the container.
   */
  function idx<Tk as arraykey, Tv>(
    ?KeyedContainer<Tk, Tv> $collection,
    ?Tk $index,
    \HH\FIXME\MISSING_PARAM_TYPE $default = null,
  )[]: Tv {}

  function idx_readonly<Tk as arraykey, Tv>(
    readonly ?KeyedContainer<Tk, Tv> $collection,
    ?Tk $index,
    \HH\FIXME\MISSING_PARAM_TYPE $default = null,
  )[]: readonly Tv {}

}
