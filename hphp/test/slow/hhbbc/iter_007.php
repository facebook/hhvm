<?php

function heh($x) { echo $x . "\n"; }

class C {
  public static function x($string) {
    foreach (self::$static_arr as $pattern => $result) {
      $pattern = 'wat' . $pattern . 'asd';
      heh($pattern);
    }
    return $string;
  }

  private static $static_arr = array(
    'a' => 'b',
    'c' => 'd',
    'e' => 'f',
    'g' => 'h',
  );
}

function main() {
  var_dump(C::x("foobarkjh1k2j3nn"));
}
main();
