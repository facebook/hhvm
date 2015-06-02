<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  ini_set('xdebug.enable', 1);
  ini_set('xdebug.default_enable', 1);
  ini_set('xdebug.overload_var_dump', 2);

  $arr = array(5);

  $b =& $arr[0];

  xdebug_var_dump($arr);
  xdebug_var_dump($b);
}

main();
