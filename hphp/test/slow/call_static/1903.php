<?php

abstract class BaseClass {
  public static function __callStatic($name,  $arguments) {
    echo "Calling static method '$name' "         . implode(', ', $arguments). "\n";
  }
}
class SubClass extends BaseClass {
}

<<__EntryPoint>>
function main_1903() {
SubClass::foo();
}
