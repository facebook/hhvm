<?php

class c {
  private static $array_or_null;
  private static $str = "asd";

  public static function go() {
    self::$array_or_null[] = 2;
    return self::$str;
  }
}

function main() {
  var_dump(c::go());
}

main();
