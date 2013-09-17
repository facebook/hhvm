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

trait T3 {
  use T2;

  public function bar() {
    echo "bar()\n";
  }
}

class C {
  use T3;
}

function foo(J $x) {
  $x->foo();
  $x->bar();
}

function main() {
  foo(new C());
}

main();
