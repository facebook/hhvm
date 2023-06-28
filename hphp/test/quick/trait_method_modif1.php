<?hh
class Foo {
  public function f() :mixed{
    return 'Foo';
  }
}
trait T {
  protected function f() :mixed{
    return 'Bar';
  }
}
class Bar extends Foo {
  use T;
  function inv() :mixed{
    return $this->f();
  }
}
<<__EntryPoint>> function main(): void {
$b = new Bar();
echo $b->inv()."\n";
}
