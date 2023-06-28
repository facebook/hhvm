<?hh

class Foo {
  protected function f() :mixed{
    return 'Foo';
  }
}
trait T {
  public function f() :mixed{
    return 'Bar';
  }
}
class Bar extends Foo {
  use T;
}

<<__EntryPoint>>
function main_2114() :mixed{
$b = new Bar();
echo $b->f()."\n";
}
