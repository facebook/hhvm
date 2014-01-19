<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

function array_cse() {
  $a = array(0,1,2,3,4);
  $x = $a[2] + $a[2];
  return $x;
}

var_dump(array_cse());
