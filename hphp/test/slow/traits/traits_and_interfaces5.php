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

trait T3 {
  use T2;

  public function bar() :mixed{
    echo "bar()\n";
  }
}

class C {
  use T3;
}

function foo(J $x) :mixed{
  $x->foo();
  $x->bar();
}

function main() :mixed{
  foo(new C());
}


<<__EntryPoint>>
function main_traits_and_interfaces5() :mixed{
main();
}
