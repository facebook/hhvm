<?hh
class Foo {
  final public function f() {
    return 'Foo';
  }
}
trait T {
  final public function f() {
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
