<?hh

class Foo { }

function run(&$b) {
  return $b;
}

$a = new Foo();
var_dump(run(&$a));
