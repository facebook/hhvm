<?php

error_reporting(-1);



class A {
  private $a = 1;
  public function __sleep() {
    return SerializationPrivateProperties::$g;
  }
  public function seta($a) { $this->a = $a; }
}
class B extends A {
  public function __sleep() {
    return SerializationPrivateProperties::$g;
  }

  static function test($a, $elems, $p = null) {

    $a->seta(42);
    SerializationPrivateProperties::$g = $elems;
    $s = serialize($a);
    var_export($s);
    echo "\n";
    $u = unserialize($s);
    var_dump($u);
    if ($p) {
      var_dump($u->$p);
    }
  }
}

B::test(new A, array("a"));
B::test(new A, array("\0A\0a"));
B::test(new A, array("\0*\0a"));
B::test(new A, array("\0*\0b"), "b");
B::test(new A, array("\0B\0b"), "b");
B::test(new A, "foo");
B::test(new B, array("a"));
B::test(new B, array("\0A\0a"));
B::test(new B, array("\0*\0a"));
B::test(new B, array("\0*\0b"), "b");
B::test(new B, array("\0B\0b"), "b");

abstract final class SerializationPrivateProperties {
  public static $g;
}
