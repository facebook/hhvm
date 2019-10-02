<?hh

function foo(inout $a) {
  var_dump($a);
}
<<__EntryPoint>> function main(): void {
foo(array()[]);
}
