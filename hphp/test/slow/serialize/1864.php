<?php

function f() {
  $a = array(123);
  $b = $a;
  $c = &$b;
  $d = new stdClass();
  $v = array(&$a, &$b, &$c, $d, $d);
  $s = serialize($v);
  echo $s;
}
f();
