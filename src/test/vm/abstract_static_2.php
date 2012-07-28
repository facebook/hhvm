<?php

abstract class Foo {
  abstract public static function who();
  public static function test() { self::who(); }
}
class Bar extends Foo {
  public static function who() { return 'Bar'; }
}

function main() {
  $bar = new Bar();
  echo $bar->test();
}

main();
