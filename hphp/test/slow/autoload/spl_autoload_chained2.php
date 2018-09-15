<?php

function my_autoload($class) {
  var_dump($class);
  if ($class == 'A')
    $test = class_exists('C');
  if ($class == 'A') {
    class A {
      public $var = 'class A';
    };
    $b = new B();
  }
  if ($class == 'B') {
    class B {
      public $var = 'class B';
    };
    $c = new C();
  }
  if ($class == 'C') {
    class C {
      public $var = 'class C';
    };
  }
}


<<__EntryPoint>>
function main_spl_autoload_chained2() {
spl_autoload_register('my_autoload');

$a = new A();
}
