<?hh
class Foo {
  final public function f() :mixed{
    return 'Foo';
  }
}
class Bar extends Foo {
  final public function f() :mixed{
    return 'Bar';
  }
}
<<__EntryPoint>> function main(): void {
$bar = new Bar();
echo $bar->f()."\n";
}
