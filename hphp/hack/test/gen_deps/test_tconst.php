<?hh // strict
class B {

}

class A {
  function foo() : B::T {
    // UNSAFE
  }

}
