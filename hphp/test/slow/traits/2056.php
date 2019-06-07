<?hh

trait T {
  abstract public function bar();
  public function foo() {
    $this->bar();
  }
}
class B {
  public function bar() {
    echo "I'm bar\n";
  }
}
class C extends B {
  use T;
}


<<__EntryPoint>>
function main_2056() {
$o = new C;
$o->foo();
}
