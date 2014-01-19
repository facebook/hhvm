<?php

class MyDerivedClass {
  public function __construct() {
    echo "__construct\n";
  }
  public function __destruct() {
    echo "__destruct\n";
  }

  public static function callNew() {
    echo "before\n";
    new self("called via PARENT");
    echo "after\n";
  }

}
$o= MyDerivedClass::callNew();

echo "Done\n";
