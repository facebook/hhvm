<?hh

trait A { public function foo() {return 1;} }
trait B { public function foo() {return 2;} }
trait C { }

class Thing {
  use A, B, C {
    A::foo insteadof B, C;
  }
}


<<__EntryPoint>>
function main_traits_and_interfaces9() {
$t = new Thing;
var_dump($t->foo());
}
