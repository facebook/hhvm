<?php

class X {
  function __destruct() { var_dump(__METHOD__); }
}

function test($a) {
  apc_store('foo', array(1 => $a));
  $a = apc_fetch('foo');
  $a[1] = 'bar';
  var_dump($a);

  $a = apc_fetch('foo');
  $a["1"] = 'bar';
  var_dump($a);

  $u = new X;
  $x =& $u;

  $a = apc_fetch('foo');
  foreach ($a as $k => $x) {
    var_dump($x, $u);
  }

  $a = apc_fetch('foo');
  $a[1] = 'bar';
  foreach ($a as $k => $x) {
    var_dump($x, $u);
  }
}

test(42);
