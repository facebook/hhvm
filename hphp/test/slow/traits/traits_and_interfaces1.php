<?hh

interface IFoo {
  public function foo();
}

trait T implements IFoo {
  public function foo() {
    echo "Hello, World!\n";
  }
}

class C {
  use T;
}

function f(IFoo $x) {
  $x->foo();
}

function main() {
  f(new C());
}


<<__EntryPoint>>
function main_traits_and_interfaces1() {
main();
}
