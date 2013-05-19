<?php

require_once "support.hhas";

class X {
  function __destruct() { var_dump(__METHOD__); }
  static function test($x) { return $x !== true; }
}

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

  var_dump(fast_array_filter($a, null));
  var_dump(fast_array_filter($a, function($a) { return !$a; }));
  var_dump(fast_array_filter($a, array('X', 'test')));

  var_dump(fast_array_map(function($a) { return $a.$a; }, $a));
  var_dump(fast_array_map(function&(&$a) { return $a; }, $a));
}

test();
