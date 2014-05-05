<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

interface IParent {
  const CBAR = 'bar';
}

abstract class CParent implements IParent {
  protected function foo(): string {
    return __CLASS__;
  }
}

abstract class Kid extends CParent {
  protected function bar() {}
}

trait KidTrait {
  require extends Kid;
  require extends CParent;

  protected function foo(): string {
    takes_parent($this);
    takes_iparent($this);
    takes_kid($this);
    takes_kt($this);
    parent::bar();
    return 'wrapped('.parent::foo().')';
  }
}

function takes_kid(Kid $p): void {}

function takes_parent(CParent $p): void {}

function takes_iparent(IParent $ip): void {}

// this declaration really should be a statically detected error,
// what with traits not being a thing at runtime ...
function takes_kt(KidTrait $kt): void {}
