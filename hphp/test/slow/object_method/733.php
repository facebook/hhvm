<?hh

class RootBase {
}
class Base extends RootBase {
  private $privateData;
}
class Test extends Base {
  protected function f1() :mixed{
    $this->privateData = 1;
    var_dump('ok');
  }
  public function f2() :mixed{
    $this->f1();
  }
}
function foo() :mixed{
  $obj = new Test();
  $obj->f2();
  $obj->privateData = 2;
  $obj = new Base();
}

<<__EntryPoint>>
function main_733() :mixed{
foo();
}
