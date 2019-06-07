<?hh

class foo {
  public function bar() {
    echo "in bar\n";
  }
  public function test() {
    self::bar();
    foo::bar();
    echo "in test\n";
  }
}

<<__EntryPoint>>
function main_727() {
$obj = new foo();
$obj->test();
}
