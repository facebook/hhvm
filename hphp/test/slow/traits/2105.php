<?hh

trait T {
  public function foo() :mixed{
  $this->bar();
  }
  private function bar() :mixed{
    echo "in bar...\n";
  }
}
class B {
 use T;
 }
class C extends B {
 }

<<__EntryPoint>>
function main_2105() :mixed{
$obj = new C;
$obj->foo();
}
