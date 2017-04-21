<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function byref(&$a) {}
function byval($a) {}

function main() {
  $a = array();
  byref($a[][0]);
  var_dump($a);
  byval($a[][0]);
}
main();
