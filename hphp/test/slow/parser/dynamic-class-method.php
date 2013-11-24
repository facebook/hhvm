<?php
class Foo {
  public static function bar($arg) {
    var_dump($arg);
  }
}
function main() {
  Foo::{'bar'}(123);
}
main();
