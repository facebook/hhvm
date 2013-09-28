<?php

trait MyTrait {
  public static function callNew() {
    new self("called via SELF");
    new parent("called via PARENT");
  }
}
class MyBaseClass {
  public function __construct($arg) {
    echo __class__ . ": " . $arg . "\n";
  }
}
class MyDerivedClass extends MyBaseClass {
  use MyTrait;
  public function __construct($arg) {
    echo __class__ . ": " . $arg . "\n";
  }
}
$o= MyDerivedClass::callNew();
