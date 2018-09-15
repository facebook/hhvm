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

function meh(): void {
  if ($v === null) {
    $outputRow[$key] = new sps_DBResultField(
      array(
        'isNull' => true,
      ));
  } else if (is_int($v)) {
    $outputRow[$key] = new sps_DBResultField(
      array(
        'isNull' => false,
        'intValue' = $v,
      ));
  }
}
