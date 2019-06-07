<?hh

trait A { public function bar() { return 1; } }

class Thing {
  use A { A::foo insteadof A; }
}


<<__EntryPoint>>
function main_traits_and_interfaces12() {
$t = new Thing;
var_dump($t->foo());
}
