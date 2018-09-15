<?php

class Base {
  public static function __callStatic($method, $args) {
    $klass = get_called_class();
    echo "class $klass\n";
  }
}
class Derived extends Base {
}

<<__EntryPoint>>
function main_1905() {
Base::foo();
Derived::foo();
}
