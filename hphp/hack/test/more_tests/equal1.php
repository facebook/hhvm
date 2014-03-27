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

interface I {}

class A implements I {}
class B implements I {}

function createFromMeta(): ?I {

  $result = null;
  switch (0) {
  case 0:
    $result = new A();
    break;
  case 1:
    $result = new B();
    break;
  }

  if ($result === null) {
    throw new Exception('Cannot create corrector %d.');
  }
  return $result;
}


