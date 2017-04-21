<?hh // decl
/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function dict<Tk, Tv>(KeyedTraversable<Tk, Tv> $arr): dict<Tk, Tv>;
function vec<Tv>(Traversable<Tv> $arr): vec<Tv>;
function keyset<Tv as arraykey>(Traversable<Tv> $arr): keyset<Tv>;
