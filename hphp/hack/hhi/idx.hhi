<?hh // decl
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/*
 * Calls to idx are rewritten by the typechecker depending on their arity. It
 * can have two signatures:
 *
 * idx<Tk, Tv>(?KeyedContainer<Tk, Tv> $collection, ?Tk $index): ?Tv
 * idx<Tk, Tv>(?KeyedContainer<Tk, Tv> $collection, Tk $index, Tv $default): Tv
 */
// NB: the typechecker relies on the exact format of this signature and rewrites
// parts of it in place during each call. Changes to the signature need to be
// done in tandem with changes to the ocaml code that munges it.
function idx<Tk, Tv>(?KeyedContainer<Tk, Tv> $collection, $index,
  $default = null) {}
