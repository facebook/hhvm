<?hh // strict

abstract class X {
  public static int $x = 4;
}

class Y extends X {
  public function f(dynamic $d): void {
    parent::$x = $d;
  }
}
