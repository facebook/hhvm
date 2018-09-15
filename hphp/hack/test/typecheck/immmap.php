<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function basic(): ImmMap<mixed, int> {
  return ImmMap { 1 => 1, 2 => 2};
}

function covariance(): ImmMap<mixed, mixed> {
  return basic();
}
