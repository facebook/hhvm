<?hh

class Foo { }

function run(inout $b) {
  return $b;
}
<<__EntryPoint>> function main(): void {
$a = new Foo();
var_dump(run(inout $a));
}
