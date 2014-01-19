<?php

class A {
  var $a;
  var $b;
}
;
function gen() {
  $obj = new A();
  var_dump($obj);
  foreach ($obj as &$value) {
    yield null;
    $value = 1;
  }
  var_dump($obj);
  $obj->c = 3;
  var_dump($obj);
  foreach ($obj as &$value) {
    yield null;
    $value = 2;
  }
  var_dump($obj);
}
foreach (gen() as $_) {
}
