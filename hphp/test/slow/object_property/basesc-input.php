<?php

class MyClass {
  protected static $stack = array();

  public function addToStack(MyClass $o)
  {
    self::$stack[] = $o;
  }
}


<<__EntryPoint>>
function main_basesc_input() {
$obj = new MyClass();
$obj->addToStack($obj);
$obj->addToStack($obj);
echo "Done\n";
}
