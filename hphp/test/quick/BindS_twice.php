<?php

class C {
  public static $blar;
  public static $blor;

  public static function buh(&$thinger = null) {
    self::$blar = &$thinger;
    self::$blor = &self::$blar;
  }
}

function main() {
  C::buh();
  var_dump(C::$blar);
  var_dump(C::$blor);
}

main();
