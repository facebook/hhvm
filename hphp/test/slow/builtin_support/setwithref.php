<?php

require_once "support.hhas";

class X {
  private $p = true;
  function __destruct() { var_dump(__METHOD__); }
  static function s_test($x) { return $x !== true; }
  function o_test($x) { return $x === $this->p; }
  function __call($n, $a) { return $a[0] == $n; }
}

function filt($x) { return $x === true; }

function test() {
  $a = array();
  var_dump(test2($a, "foo", new X));

  $a = array();
  var_dump(test2($a, "foo", "bar"));

  $a = array();
  $b = "bar";
  var_dump(test3($a, $b));

  $a = array(1, 'foo' => 'bar', 0, true, false);
  $b =& $a['foo'];

  var_dump("No function", fast_array_filter($a, null));
  var_dump("Closure", fast_array_filter($a, function($a) { return !$a; }));
  var_dump("Static Method", fast_array_filter($a, array('X', 's_test')));
  var_dump("Dynamic Method", fast_array_filter($a, array(new X, 'o_test')));
  var_dump("__call", fast_array_filter($a, array(new X, 'bar')));
  var_dump("string", fast_array_filter($a, 'filt'));

  var_dump(fast_array_map(function($a) { return $a.$a; }, $a));
  var_dump(fast_array_map(function&(&$a) { return $a; }, $a));
}

test();
