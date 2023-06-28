<?hh
trait T {
  final public function foo() :mixed{
    return 'T::foo';
  }
}
class Foo {
  use T;
}
class Bar extends Foo {
  final public function foo() :mixed{
    return 'Bar::foo';
  }
}
<<__EntryPoint>> function main(): void {
$bar = new Bar();
echo $bar->foo()."\n";
}
