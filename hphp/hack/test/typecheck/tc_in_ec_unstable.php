<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

// expected error because the file attribute 'enum_class_type_constants'
// is missing
enum class E : int {
  const type T = int;
}

abstract enum class A : mixed {
 abstract const type T;
}
