<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

// Allow delayed implementation for abstract classes

interface IFace {
  public function foo(): int;
}

// multiple layers
abstract class AClass implements IFace {
}

abstract class BClass extends AClass {}
abstract class CClass extends BClass {}
class DClass extends CClass { public function foo(): int { return 0; }}
