<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function f1(): string {
  $x = new SomeUnknownClass();
  return $x['foo'];
}

function f2() {
  $x = new SomeUnknownClass();
  $x[] = 'juju';
}
