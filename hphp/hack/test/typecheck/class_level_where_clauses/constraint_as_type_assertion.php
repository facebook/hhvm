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

abstract class Test<T> where T as num {
  abstract public function get(): T;
  public function return_num(): num {
    return $this->get();
  }
}
