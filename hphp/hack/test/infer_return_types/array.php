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

function f() {
  return array(
    'a' => 1,
  );
}

async function g() {
  return array(
    'a' => 1,
  );
}

function h() {
  return array(1);
}

function i() {
  return array(
    'a' => 1,
    'b' => false,
  );
}

function j() {
  return array(1, false);
}

function k() {
  $a = array();
  $a['a'] = 4;
  return $a;
}
