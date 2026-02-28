<?hh

class X {
  protected function foo() :mixed{ echo "X::foo\n"; }
  private function bar() :mixed{ echo "X::bar\n"; }
  protected $field = 1;
}


class A extends X {
  protected function foo() :mixed{
    echo "A::foo " . $this->field . "\n";
  }
}

class B extends X {
  function foo() :mixed{
    $a = new A;
    $a->foo();
    $a->field = 123;
    $a->foo();
  }
}
<<__EntryPoint>> function main(): void {
$b = new B;
$b->foo();
}
