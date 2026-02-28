<?hh

trait T {
  abstract public function bar():mixed;
  public function foo() :mixed{
    $this->bar();
  }
}
class B {
  public function bar() :mixed{
    echo "I'm bar\n";
  }
}
class C extends B {
  use T;
}


<<__EntryPoint>>
function main_2056() :mixed{
$o = new C;
$o->foo();
}
