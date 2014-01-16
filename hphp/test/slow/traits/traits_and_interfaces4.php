<?hh

interface I {
  public function foo();
}

interface J extends I {
  public function bar();
}

trait T1 implements J {
  public function foo() {
    echo "foo()\n";
  }
}

trait T2 {
  use T1;
}

class C {
  use T2;
}

function foo(J $x) {
  $x->foo();
  $x->bar();
}

function main() {
  foo(new C());
}

main();
