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

// Class-level where constraints on generic T
class Base <T> where T as num {}
class Num_ <Tu> extends Base<Tu> where Tu = int {}     // OK!
