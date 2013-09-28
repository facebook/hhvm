<?php

abstract class BaseClass {
}
class SubClass extends BaseClass {
  public static function __callStatic($name,  $arguments) {
    echo "Calling static method '$name' "         . implode(', ', $arguments). "\n";
  }
}
SubClass::foo();
