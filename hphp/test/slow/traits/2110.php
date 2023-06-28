<?hh

trait T {
  private function foo() :mixed{
}
}
class C {
  protected function foo() :mixed{
}
}
class D extends C {
  use T;
  protected function foo() :mixed{
}
}
class E extends C {
  public static function test($obj) :mixed{
    $obj->foo();
  }
}

<<__EntryPoint>>
function main_2110() :mixed{
$d = new D;
E::test($d);
echo "Done\n";
}
