<?php
class A {

  public static function factory() {
    return new A;
  }

  public static function b() {
    return 'no segfault';
  }

  public function __destruct() {
  }
}

function main() {
  echo A::factory()->b();
}
main();
