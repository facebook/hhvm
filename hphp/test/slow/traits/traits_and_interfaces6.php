<?hh

interface I {
  public function foo():mixed;
  public function bar():mixed;
}

trait T implements I {
  public function foo() :mixed{
    echo "T::foo\n";
  }
  public function bar() :mixed{
    echo "T::bar\n";
  }
}

class C implements I {
  use T;

  public function bar() :mixed{
    echo "C::bar\n";
  }
}

function xyz(I $x) :mixed{
  $x->foo();
  $x->bar();
}

function main() :mixed{
  $c = new C();
  xyz($c);
}


<<__EntryPoint>>
function main_traits_and_interfaces6() :mixed{
error_reporting(-1);

main();
}
