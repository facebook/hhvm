<?hh

class Foo {
  private function f() :mixed{
    return 'Foo';
  }
}
trait T {
  private function f() :mixed{
    return 'Bar';
  }
}
class Bar extends Foo {
  use T;
  function inv() :mixed{
    return $this->f();
  }
}

<<__EntryPoint>>
function main_2115() :mixed{
$b = new Bar();
echo $b->inv()."\n";
}
