<?hh

trait T {}

interface I {}

abstract class A {
    use T;
}

class B extends A implements I {}
