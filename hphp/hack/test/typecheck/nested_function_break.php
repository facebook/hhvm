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

function f(): (function (): void) {
  while (true) {
    return function () {
      break;
    };
  } // FIXME(coeffects) below should be also OK without [defaults]
  return function ()[defaults] {};
}
