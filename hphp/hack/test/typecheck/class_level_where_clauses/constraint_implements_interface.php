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

abstract class Test implements I {}  // OK!

class Bad implements I {}            // Error!

interface I where this as Test {}

// interface J implements I where this as Test {}

interface K {
  require extends Test;
}
