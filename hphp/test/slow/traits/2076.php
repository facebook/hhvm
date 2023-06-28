<?hh

trait U {
  public function test() :AsyncGenerator<mixed,mixed,void>{
    echo __CLASS__ . "\n";
    $this->foo();
    yield null;
  }
}
class D {
  use U;
  protected function foo() :mixed{
    echo "U::foo\n";
  }
}

<<__EntryPoint>>
function main_2076() :mixed{
$obj = new D;
$x = $obj->test();
foreach ($x as $v) {
}
}
