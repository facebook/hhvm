<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function byref(&$a) {}

<<__EntryPoint>>
function main() {
  $a = array();
  $a[] = array();
  byref(&$a[0][0]);
  var_dump($a);
}
