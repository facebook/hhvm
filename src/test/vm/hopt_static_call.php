<?php
class AA {
  public static function f1($a1) { print "Pass\n"; }
  public static function f0($a1) { AA::f1($a1); }
}
AA::f0("Hello World\n");
