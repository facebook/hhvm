<?hh

trait T {
  public function foo() {
  $this->bar();
  }
  private function bar() {
    echo "in bar...\n";
  }
}
class B {
 use T;
 }
class C extends B {
 }

<<__EntryPoint>>
function main_2105() {
$obj = new C;
$obj->foo();
}
