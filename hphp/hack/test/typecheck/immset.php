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

function basic(): ImmSet<int> {
  return ImmSet { 1 };
}

function covariance(): ImmSet<arraykey> {
  return basic();
}
