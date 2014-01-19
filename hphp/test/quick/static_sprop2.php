<?php
class C {
  private static $cls = 'C';
  public function __construct() {
    var_dump(static::$cls);
  }
}

class D extends C {
  private static $cls = 'D';
  public function __construct() {
    parent::__construct();
    var_dump(static::$cls);
  }
}

echo "Creating C\n";
$c = new C;
echo "Creating D\n";
$d = new D;
echo "**************\n";

