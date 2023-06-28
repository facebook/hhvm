<?hh

class C {
  protected function foo() :mixed{
 echo "C::foo\n";
 }
}
trait T {
  protected function foo() :mixed{
 echo "T::foo\n";
 }
}
class D extends C {
  use T;
}
class E extends C {
  public static function test($obj) :mixed{
    $obj->foo();
  }
}

<<__EntryPoint>>
function main_2111() :mixed{
$d = new D;
E::test($d);
echo "Done\n";
}
