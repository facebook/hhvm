<?php
for ($i = 0; $i < 3; $i++) {
  $b = 30;
  $a =& $b;
  $b = 60;
  var_dump($a, $b);
  $a = "foo";
  var_dump($a, $b);
  unset($a);
  unset($b);
}
for ($i = 0; $i < 2; $i++) {
  $b = array(1, 2, 3);
  $a =& $b;
  $b = 60;
  var_dump($a, $b);
  $a = "foo";
  var_dump($a, $b);
}
function f() {
  $b = 30;
  $a =& $b;
  $b = array();
  $a =& $b;
  $b = 60;
  var_dump($a);
}
f();
