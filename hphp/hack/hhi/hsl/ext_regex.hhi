<?hh // decl
/**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH\Lib\Regex {
  type Match = shape(...);
  newtype Pattern<+T as Match> = string;
}
