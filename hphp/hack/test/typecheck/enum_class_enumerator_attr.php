<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

// Attribuets are allowed on type constants, but not on enumerators

enum class E : int {
  <<__Enforceable>> // allowed
    const type T = int;
  <<__Enforceable>> // not allowed
    int A = 42;
}
