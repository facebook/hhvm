<?php

abstract class BaseClass {
  public static function __callStatic($name,  $arguments) {
    echo "Calling BaseClass static method '$name' "         . implode(', ', $arguments). "\n";
  }
}
class SubClass extends BaseClass {
  public static function __callStatic($name,  $arguments) {
    echo "Calling SubClass static method '$name' "         . implode(', ', $arguments). "\n";
  }
}
SubClass::foo();
BaseClass::foo();
