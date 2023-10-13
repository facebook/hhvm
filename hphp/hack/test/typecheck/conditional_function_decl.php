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

if (true) {
  function f(): void {} // bad
} else {
  function g(): void {} // bad
}

class ConditionalDecl {
  function h(): void {} // good
}
