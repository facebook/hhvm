<?hh // partial
/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

abstract final class dict<+Tk, +Tv> implements Indexish<Tk, Tv> {}
abstract final class keyset<+T as arraykey> implements Indexish<T, T> {}
abstract final class vec<+T> implements Indexish<int, T> {}
