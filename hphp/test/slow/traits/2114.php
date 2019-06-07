<?hh

class Foo {
  protected function f() {
    return 'Foo';
  }
}
trait T {
  public function f() {
    return 'Bar';
  }
}
class Bar extends Foo {
  use T;
}

<<__EntryPoint>>
function main_2114() {
$b = new Bar();
echo $b->f()."\n";
}
