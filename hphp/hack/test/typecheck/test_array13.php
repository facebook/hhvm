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

function test(): void {
  $v = dict['f' => vec[1, 2], 'b' => 0];
  $v['f'][0] = 1;
}
