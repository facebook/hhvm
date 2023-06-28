<?hh

trait my_trait {
  abstract function foo():mixed;
  public function bar() :mixed{
    echo "I am bar\n";
    self::foo();
  }
}
class my_class {
  use my_trait;
  private function foo() :mixed{
    echo "I am foo\n";
  }
}

<<__EntryPoint>>
function main_2054() :mixed{
$o = new my_class;
$o->bar();
}
