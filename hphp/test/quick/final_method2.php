<?hh
class Foo {
  final public function f() :mixed{
    return 'Foo';
  }
}
trait T {
  final public function f() :mixed{
    return 'Bar';
  }
}
class Bar extends Foo {
  use T;
}
<<__EntryPoint>> function main(): void {
$bar = new Bar();
echo $bar->f()."\n";
}
