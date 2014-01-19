<?php

function __autoload($name) {
  switch ($name) {
    case 'C':      class C {
}
      break;
    case 'M':      class M {
 function foo() {
}
 }
      break;
    default: class C{
}
 class M {
}
  }
  var_dump($name);
}
$r1 = new ReflectionClass('C');
$r2 = new ReflectionMethod('M', 'foo');
