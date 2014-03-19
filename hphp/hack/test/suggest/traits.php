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

trait MyTrait {
  // We don't currently do a good job inferring the types of member variables
  // defined in traits. This test is currently just making sure we don't totally
  // explode.
  protected $x;
}

class C {
  use MyTrait;
}
