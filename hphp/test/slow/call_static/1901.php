<?php

abstract class BaseClass {
  public static function __callStatic($name,  $arguments) {
    echo "Calling static method '$name' "         . implode(', ', $arguments). "\n";
  }
}

<<__EntryPoint>>
function main_1901() {
BaseClass::foo();
}
