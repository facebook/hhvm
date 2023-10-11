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

function f(darray<string, bool> $arr): void {
  switch (1) {
    case 1:
      if ($arr['foo']) {
      }
      break;
    case 2:
      $arr = false;
      break;
  }
}
