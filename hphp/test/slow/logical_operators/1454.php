<?php

function f($a) {
 var_dump('f:'.$a);
 return $a;
 }
function foo($a) {
  var_dump($a && true);
  var_dump(f($a) && true);
  var_dump(true && $a);
  var_dump(true && f($a));
  var_dump($a && false);
  var_dump(f($a) && false);
  var_dump(false && $a);
  var_dump(false && f($a));
  var_dump($a || true);
  var_dump(f($a) || true);
  var_dump(true || $a);
  var_dump(true || f($a));
  var_dump($a || false);
  var_dump(f($a) || false);
  var_dump(false || $a);
  var_dump(false || f($a));
}
foo(34);
foo(0);
