<?php

class A {
  static function bar(&$a) {
    $a = 'ok';
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

<<__EntryPoint>>
function main_1478() {
$a = 'failed';
A::bar(&$a);
var_dump($a);
if (false) {
  include '1478.inc';
}
$a = 'failed';
A2::bar(&$a);
var_dump($a);
}
