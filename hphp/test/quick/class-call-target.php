<?hh

abstract class A {
  abstract function foo(): int;

  function bar(): int {
    return $this->foo();
  }
}

abstract class B extends A {
  function baz(): int {
    return $this->bar();
  }
}

class B1 extends B {
  <<__Override>>
  function foo(): int {
    return 10;
  }
}

class B2 extends B {
  <<__Override>>
  function foo(): int {
    return 11;
  }
}

class C1 extends A {
  <<__Override>>
  function foo(): int {
    return 100;
  }
}

class C2 extends A {
  <<__Override>>
  function foo(): int {
    return 110;
  }
}

function bar(A $a): void {
  var_dump($a->bar());
}

function fox(B $b): int {
  return $b->baz();
}

<<__EntryPoint>>
function main() {
  bar(new C1());
  bar(new C2());
  bar(new B1());
  fox(new B1());
}
