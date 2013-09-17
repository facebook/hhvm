<?hh
interface IFoo {
  public function foo();
}

trait T1 implements IFoo {}

trait T2 {
  use T1;

  public function foo() {
    echo "Hello, World!\n";
  }
}

class C { use T2; }

function f(IFoo $x) {
  $x->foo();
}

function main() {
  f(new C());
}

main();
