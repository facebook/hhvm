<?hh

class A {
  function __construct() {
    throw new Exception();
  }
}

class B {
}

function foo($a, $b) {
}

function bar($a, $b) {
}
<<__EntryPoint>> function main(): void {
bar(new B, foo(1, new A));
}
