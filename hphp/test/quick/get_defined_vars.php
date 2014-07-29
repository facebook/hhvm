<?php

function f($a=500, $b) {
  // $b shouldn't show up
  var_dump(get_defined_vars());
}

function g() {
  $usevar = 0;
  $closure = function($arg1) use ($usevar) {
    $local = 0;
    var_dump(get_defined_vars());
  };
  $closure(1);
}

function modify($arr) {
  $arr['a'] += 200;
}

function rvalue_sort($x) { ksort($x); return $x; }

function ref($a) {
  $b =& $a;
  var_dump(rvalue_sort(get_defined_vars()));
  modify(rvalue_sort(get_defined_vars()));
  var_dump($b);
}

@f();
g();
ref(20);
