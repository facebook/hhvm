<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function main($o) {
  $x =& $y;
  $x = $o->prop;
  return isset($x[23]) ? true : false;
}

$o = new stdclass;
$o->prop = array(23 => 'hi');
echo main($o) ? "true\n" : "false\n";
