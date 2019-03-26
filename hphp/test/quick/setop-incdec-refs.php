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

function elements(&$v, $a) {
  $v = 'a string';
  var_dump($a);
  $x = $a['ref'] .= ' tail';
  var_dump($a);
  var_dump($v);

  $x = $a['ref']++;
  var_dump($a);
  var_dump($v);
}

<<__EntryPoint>>
function main() {
  $c = new stdclass;
  properties(&$c->foo, $c);
  $a = array();
  elements(&$a['ref'], $a);
}
