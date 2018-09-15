<?php

class Foo {
  static function Bar() {
    if (isset($this) && isset($this->bar)) {
      echo "isset\n";
    }
    var_dump($this);
  }
}

 <<__EntryPoint>>
function main_738() {
Foo::Bar();
 $obj = new Foo();
 $obj->Bar();
}
