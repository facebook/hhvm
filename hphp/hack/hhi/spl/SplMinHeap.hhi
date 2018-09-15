<?hh // decl /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class SplMinHeap<T> extends SplHeap<T> {

  // Methods
  protected function compare(T $value1, T $value2): int;
}
