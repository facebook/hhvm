<?php
class AA {
  public $aa1 = 0;
  public static function f1() { print "Pass\n"; return 1; }
  // public static function f0() {  AA::f1();  }
  public static function f0() {  if (AA::f1()) {  echo "aa1 != 0\n";  }  }
}
AA::f0();
