<?hh

interface I {
  public function foo():mixed;
}

interface J extends I {
  public function bar():mixed;
}

trait T1 implements J {
  public function foo() :mixed{
    echo "foo()\n";
  }
}

trait T2 {
  use T1;
}

class C {
  use T2;
}

function foo(J $x) :mixed{
  $x->foo();
  $x->bar();
}

function main() :mixed{
  foo(new C());
}


<<__EntryPoint>>
function main_traits_and_interfaces4() :mixed{
main();
}
