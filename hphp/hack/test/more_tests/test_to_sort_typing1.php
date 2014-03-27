<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */






/*
function f($y, $z){
  $y = $y . $z;
  $z[0] = 'dd' + 1;
  return $y;
}
*/

/*
function g($x) {
  if (true) {
    $x = array('d' => 'f');
    return $x;
  }
  $x['dd'] = 'dd' + $x['dd'];
  return $x;
}


*/

/*
function g($x) {
  $x = array('d' => 0);
  $x['dd'] = 'dd' + $x['dd'];
  return $x;
}
*/



function extended_friend_cachekey_scb_used_apps($update_params) {
  $x = $update_params[0];
  $y = $update_params + 1;
}

/*
class X {

  public function g(){
    $y = array();
    $x = 0;
    return $x[1];

  }
}

*/
