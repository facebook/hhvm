<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

function main() {
  $a = null;
  unset($a['foo']);
  unset($o->foo['blah']);
  unset($o['basd']['sadfsadf']);
  return $a;
}
function main2() {
  $a = 'hello';
  unset($a['bar']);
}
var_dump(main());
main2();

