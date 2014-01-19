<?php

interface I {
 public function test($a);
}
class A implements I {
 public function test($a) {
 print $a;
}
}
$obj = new A();
 var_dump($obj instanceof I);
 $obj->test('cool');
