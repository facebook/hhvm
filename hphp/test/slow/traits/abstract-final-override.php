<?php

trait T {
    final function foo() {}
}

trait X {
    abstract function foo() ;
}

interface I1 {}
interface I2 {}

class A implements I1 {
    use T;
}

if (isset($g)) {
  class A {}
}

abstract class B extends A implements I2 {
    use X;
}

class C extends B {}

var_dump(new C);
