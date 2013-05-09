<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

function f() {
  $b = 30;
  $a =& $b;
//  $b = array();
//  $a =& $b;
//  $b = 60;
//  var_dump($a);
  echo $a;
}
f();
