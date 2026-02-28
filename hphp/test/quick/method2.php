<?hh

class X {
  protected function foo() :mixed{
    echo "X::foo\n";
  }
  private function bar() :mixed{
    echo "X::bar\n";
  }
  private function baz() :mixed{
    echo "X::baz\n";
  }
}

class Y extends X {
  protected function bar() :mixed{
    echo "Y::bar\n";
  }
}

class A extends Y {
  protected function foo() :mixed{
    echo "A::foo\n";
  }
  protected function bar() :mixed{
    echo "A::bar\n";
  }
  protected function baz() :mixed{
    echo "A::bar\n";
  }
}

class B extends Y {
  function foo() :mixed{
    $a = new A;
    $a->foo();
    $a->bar();
//    $a->baz();
  }
}
<<__EntryPoint>> function main(): void {
$b = new B;
$b->foo();
}
