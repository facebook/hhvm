<?hh

interface IFoo {
  public function foo():mixed;
}

trait T implements IFoo {
  public function foo() :mixed{
    echo "Hello, World!\n";
  }
}

class C {
  use T;
}

function f(IFoo $x) :mixed{
  $x->foo();
}

function main() :mixed{
  f(new C());
}


<<__EntryPoint>>
function main_traits_and_interfaces1() :mixed{
main();
}
