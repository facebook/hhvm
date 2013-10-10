<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function main($a) {
  $x =& $y;
  $x = $a[0];
  return empty($x) ? true : false;
}

echo main(array(array()))."\n";
