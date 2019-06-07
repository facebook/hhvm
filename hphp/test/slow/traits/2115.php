<?hh

class Foo {
  private function f() {
    return 'Foo';
  }
}
trait T {
  private function f() {
    return 'Bar';
  }
}
class Bar extends Foo {
  use T;
  function inv() {
    return $this->f();
  }
}

<<__EntryPoint>>
function main_2115() {
$b = new Bar();
echo $b->inv()."\n";
}
