<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */
abstract class C {};

interface IThis {
  public function get(): this;
  public function set(this $v): void;
}

interface IC extends IThis where this = C {
  public function get(): C;
  public function set(C $v): void;
}
