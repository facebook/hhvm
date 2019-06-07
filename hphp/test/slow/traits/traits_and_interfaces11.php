<?hh

trait A { public function foo() { return 1; } }
trait B { public function foo() { return 2; } }

class Thing {
  use A, B { B::foo insteadof A; }
  // This would throw "unknown trait A" if B were not present in the insteadof.
  use A, B { B::foo insteadof A, B; }
}


<<__EntryPoint>>
function main_traits_and_interfaces11() {
$t = new Thing;
var_dump($t->foo());
}
