<?php
class C {
  public function __call($fn, $args) {
    echo "C::__call\n";
    var_dump(isset($this));
    var_dump($fn, $args);
    echo "\n";
  }
  public static function test() {
    // FPushClsMethodD
    C::foo("a", "b", "c", "d");

    // FPushClsMethod
    $cls = 'C';
    $cls::foo("a", "b", "c", "d");
    $fn = 'foo';
    C::$fn("a", "b", "c", "d");
    $fn = 'foo';
    $cls::$fn("a", "b", "c", "d");

    // FPushClsMethodF
    self::foo("a", "b", "c", "d");
  }
}

function main() {
  $obj = new C;
  $obj->test();
}

main();

