<?php

class C {
 }
function foo($p) {
  if ($p) {
    $obj = new C;
  }
 else {
    $a = array(1);
  }
  var_dump($obj == $a);
}
foo(false);
