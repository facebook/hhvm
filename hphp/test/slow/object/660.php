<?php

class A {
  var $a;
  var $b;
}
;
function gen() {
  $obj = new A();
  $obj2 = $obj;
  foreach ($obj2 as $k => &$value) {
    yield null;
    $value = 'ok';
  }
  var_dump($obj);
  var_dump($obj2);
}
foreach (gen() as $_) {
}
