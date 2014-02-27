<?php

class A { public function bummer($x) {var_dump($x);} }
class B { public function bummer(&$x) {$x = "asd"; var_dump($x);} }

class Foob {
  private static $maybe_boxed = 0;
  private static $ok = "ok";

  public static function a($y) {
    $y->bummer(self::$maybe_boxed);
  }
  public static function get() { return self::$maybe_boxed; }
  public static function ok() { return self::$ok; }
}

function ok() { return Foob::ok(); }

function main() {
  var_dump(Foob::get());
  Foob::a(new A());
  var_dump(Foob::get());
  Foob::a(new B());
  var_dump(Foob::get());
  var_dump(Foob::ok());
  var_dump(ok());
}

main();
