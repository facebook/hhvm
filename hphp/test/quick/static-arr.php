<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  $x = array();
  $x += array(1, 2, 3);
  var_dump($x);

  $s = 'hi';
  $s .= '1234556';
  var_dump($s);
}
main();
