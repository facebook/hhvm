<?hh // strict

class C {
  public static ?(function (int): void) $f = null;
}

class D extends C {
  public function f(dynamic $d): void {
    $z = self::$f;
    if ($z !== null) {
      $z($d); // error
    }
  }
}
