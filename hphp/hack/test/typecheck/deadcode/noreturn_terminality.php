<?hh

class A {
  public function foo(): noreturn {
    throw new Exception();
  }
}

function bar(): noreturn {
  throw new Exception();
}

function test1(): noreturn {
  bar();
}

function test2(A $x): noreturn {
  $x->foo();
}
