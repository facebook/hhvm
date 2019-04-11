<?php
// Copyright 2004-2015 Facebook. All Rights Reserved.

function properties(&$a, $c) {
  $a = 5;
  var_dump($c);
  $x = $c->foo += 10;
  var_dump($c);
  var_dump($a);

  $x = --$c->foo;
  var_dump($c);
  var_dump($a);
}

<<__EntryPoint>>
function main() {
  $c = new stdclass;
  properties(&$c->foo, $c);
}
