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

class A {}

function test(string $field1): void {
  $v = dict[$field1 => 1, 'field2' => new A()];
  $v[$field1]->unknownFunction();
}
