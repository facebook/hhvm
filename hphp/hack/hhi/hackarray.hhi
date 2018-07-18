<?hh // partial
/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

abstract final class dict<+Tk as arraykey, +Tv> implements Indexish<Tk, Tv>, XHPChild {}
abstract final class keyset<+T as arraykey> implements Indexish<T, T>, XHPChild {}
abstract final class vec<+T> implements Indexish<int, T>, XHPChild {}
