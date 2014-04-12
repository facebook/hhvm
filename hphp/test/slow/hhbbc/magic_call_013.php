<?php

class Base {
  private static function hehe() {
    return "hehe";
  }
}

class D1 extends Base {
  public static function __callStatic($x, $y) {
    return "__callStatic";
  }

  static function yo() {
    return static::hehe();
  }
}

function main() {
  $y = D1::yo();
  echo "called\n";
  var_dump($y);
}

main();
