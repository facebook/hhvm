<?hh

trait T {
  final public function foo() :mixed{
    return 'Bar';
  }
}
class Bar {
  use T;
  final public function foo() :mixed{
    return 'Foo';
  }
}

<<__EntryPoint>>
function main_2113() :mixed{
$bar = new Bar();
echo $bar->foo()."\n";
}
