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

interface X {
  const int V = 0;
}

class Y implements X {
  public function foo(): string {
    return X::V;
  }
}
