<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function go($a, $r) {
  if ($r) $a =& $a;
  if ($a) $x = 5;

  $a[1]['hello'] = 5;
  if (isset($a[2]['hi'])) return true;
  return false;
}

$a = array();
var_dump(go($a, false));
var_dump(go($a, true));
