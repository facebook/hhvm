<?php

class A {
  var $a;
  var $b;
}

<<__EntryPoint>>
function main_659() {
;
$obj = new A();
$obj2 = $obj;
foreach ($obj2 as $k => &$value) {
  $value = 'ok';
}
var_dump($obj);
var_dump($obj2);
}
