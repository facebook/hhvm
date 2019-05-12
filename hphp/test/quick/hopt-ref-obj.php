<?hh

class Foo { }

function run(&$b) {
  return $b;
}
<<__EntryPoint>> function main(): void {
$a = new Foo();
var_dump(run(&$a));
}
