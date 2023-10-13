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

async function a(): Awaitable<int> {
  return 1;
}

function f(): int {
  return a() ? 1 : 2;
}
