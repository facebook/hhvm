<?php

function test() {
  class testCore {
    public function __construct() {
      var_dump("myClass");
    }
  }
}

class ClassAutoloader {
  public function __construct() {
    spl_autoload_register(array($this, 'loader'));
  }
  private function loader($className) {
    echo 'Trying to load ', $className, ' via ', __METHOD__, "()\n";
    if ($className == "testCore") {
      var_dump("myClass expect");
      test();
    }
  }
}


<<__EntryPoint>>
function main_spl_autoload_register_class_private_array_object() {
$autoloader = new ClassAutoloader();
$obj = new testCore();
}
