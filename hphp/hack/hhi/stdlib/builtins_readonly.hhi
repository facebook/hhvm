<?hh
<<file:__EnableUnstableFeatures("readonly")>>
/**
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH\Readonly {
  // Converts a readonly primitive or value type into a mutable one.
  // Only works on non-collection value types that are not affected by readonly,
  // like int, string, etc. Currently only enforced by the typechecker.
  function as_mut<T>(readonly T $x)[]: T;
}
