<?hh

class A {
  public function foo() :mixed{
    print "A::foo\n";
    $this->bar();
  }
  protected function baz() :mixed{
    print "A::baz\n";
  }
  private function f() :mixed{
    echo "A::f\n";
  }
  public function g($a) :mixed{
    echo "A::g\n";
    $a->f();
  }
}

class B extends A {
  protected function bar() :mixed{
    print "B::bar\n";
    $this->baz();
  }
  public function h($a) :mixed{
    print "B::g\n";
    $a->f();
  }
}
<<__EntryPoint>> function main(): void {
$a = new A;
$b = new B();
$b->foo();
$b->g($a);
//$b->h($a);
}
