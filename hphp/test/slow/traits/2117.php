<?hh

class Foo {
  protected function f() {
    return 'Foo';
  }
}
trait T {
  protected function f() {
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
function main_2117() {
$b = new Bar();
echo $b->inv()."\n";
}
