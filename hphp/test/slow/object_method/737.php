<?hh

class B {
  public function foo() :mixed{
    $this->bar();
  }
  private function bar() :mixed{
    var_dump('in B::bar...');
  }
}
class C extends B {
  private function bar() :mixed{
    var_dump('in C::bar!');
  }
}

<<__EntryPoint>>
function main_737() :mixed{
$obj = new C;
$obj->foo();
}
