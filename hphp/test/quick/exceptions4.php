<?hh

class A {
  function __construct() {
    throw new Exception();
  }
}

class B {
}

function foo($a, $b) :mixed{
}

function bar($a, $b) :mixed{
}
<<__EntryPoint>> function main(): void {
bar(new B, foo(1, new A));
}
