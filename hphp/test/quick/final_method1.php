<?hh
class Foo {
  final public function f() {
    return 'Foo';
  }
}
class Bar extends Foo {
  final public function f() {
    return 'Bar';
  }
}
<<__EntryPoint>> function main(): void {
$bar = new Bar();
echo $bar->f()."\n";
}
