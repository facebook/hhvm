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
function f($x){
  $x = null;
  if(true){
    $x = array(0);
  }
  if($x){
    $x[0] = 1;
    return $x;
  }
}
*/
/*
function g($x){
  $x['d'];
  
  if (true){
    $x['h'] = 1;
  }
  
  return $x;
}
*/
/*
function g($x){
  $x['d'];
  
  if (true){
    $x['h'] = 1;
  } else {
    $x['h'] = 0;
  }
  
  return $x;
}
*/


/*
function g($x){
  $x['d'] = 0;

  $y = array('d' => 0, 'foo' => 1);
  if(true){
    return $x;
  }
  return $y;
}
*/

function g($x){
  $x = array('gg' => 0);

  if (true) {
    $x['d'] = 0;
  } 
  
  return $x;
}
