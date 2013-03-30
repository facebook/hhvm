<?php

class AA {
    function AA() { echo "foo\n"; }
}
class bb extends AA {}
class CC extends bb {
   function CC() { parent::bb(); }
}
new CC();

class A {
    function A() { echo "bar\n"; }
}
class B extends A {}
class C extends B {
   function C() { parent::B(); }
}
new C();

?>