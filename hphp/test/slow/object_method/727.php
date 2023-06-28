<?hh

class foo {
  public function bar() :mixed{
    echo "in bar\n";
  }
  public function test() :mixed{
    self::bar();
    foo::bar();
    echo "in test\n";
  }
}

<<__EntryPoint>>
function main_727() :mixed{
$obj = new foo();
$obj->test();
}
