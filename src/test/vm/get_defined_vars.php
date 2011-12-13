<?php

function f($a=500, $b) {
  // $b shouldn't show up
  var_dump(get_defined_vars());
}

function modify($arr) {
  $arr['a'] += 200;
}

function ref($a) {
  $b =& $a;
  var_dump(get_defined_vars());
  modify(get_defined_vars());
  var_dump($b);
}

@f();
ref(20);
