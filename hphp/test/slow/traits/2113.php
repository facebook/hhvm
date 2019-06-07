<?hh

trait T {
  final public function foo() {
    return 'Bar';
  }
}
class Bar {
  use T;
  final public function foo() {
    return 'Foo';
  }
}

<<__EntryPoint>>
function main_2113() {
$bar = new Bar();
echo $bar->foo()."\n";
}
