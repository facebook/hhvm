<?php

trait TestTrait {
  public static function __callStatic($name, $arguments) {
    echo "__callStatic: " . $name . "\n";
  }
}

class A {
  use TestTrait;
}

function main() {
  call_user_func('A::Test');
  call_user_func(array('A','Test'));

  $obj = new A;
  call_user_func(array($obj, 'Test'));
}
main();

