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

<<__EntryPoint>>
function main_static_props_003() {
var_dump(Foob::asd());
var_dump(Foob::wat());
}
