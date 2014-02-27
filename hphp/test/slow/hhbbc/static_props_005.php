<?php

class Foob {
  private static $heh = 0;

  public static function asd() {
    self::$heh++;
    return self::$heh;
  }
}
var_dump(Foob::asd());
var_dump(Foob::asd());
var_dump(Foob::asd());
