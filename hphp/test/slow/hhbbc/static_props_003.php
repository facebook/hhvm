<?php

class Foob {
  private static $a;
  private static $heh = "asd";

  public static function asd() {
    $x =& self::$a;
    $x = "test";
    return self::$heh;
  }
  public static function wat() {
    return self::$a;
  }
}
var_dump(Foob::asd());
var_dump(Foob::wat());
