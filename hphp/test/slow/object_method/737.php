<?hh

class B {
  public function foo() {
    $this->bar();
  }
  private function bar() {
    var_dump('in B::bar...');
  }
}
class C extends B {
  private function bar() {
    var_dump('in C::bar!');
  }
}

<<__EntryPoint>>
function main_737() {
$obj = new C;
$obj->foo();
}
