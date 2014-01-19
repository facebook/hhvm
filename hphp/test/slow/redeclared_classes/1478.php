<?php

class A {
  static function bar(&$a) {
    $a = 'ok';
  }
}
$a = 'failed';
A::bar($a);
var_dump($a);
if (false) {
  class A{
}
  class A2{
}
}
class C {
  static function bar() {
  }
}
class A2 extends C {
  static function bar(&$a) {
    $a = 'ok';
  }
}
$a = 'failed';
A2::bar($a);
var_dump($a);
