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

class Equal<T1 as T2, T2 as T1> extends Equal<T2, T1> {

  public function __construct(T1 $x, T2 $y) {
    parent::__construct($x, $y);
  }
}
