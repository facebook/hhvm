<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

async function array_return(
  array<Awaitable<int>> $gens,
): Awaitable<array<int>> {
  $ret = await gena($gens);
  return $ret;
}
