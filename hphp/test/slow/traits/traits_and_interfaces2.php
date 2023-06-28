<?hh
interface IFoo {
  public function foo():mixed;
}

trait T1 implements IFoo {}

trait T2 {
  use T1;

  public function foo() :mixed{
    echo "Hello, World!\n";
  }
}

class C { use T2; }

function f(IFoo $x) :mixed{
  $x->foo();
}

function main() :mixed{
  f(new C());
}


<<__EntryPoint>>
function main_traits_and_interfaces2() :mixed{
main();
}
