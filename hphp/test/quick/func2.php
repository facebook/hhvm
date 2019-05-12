<?hh

function foo(&$a) {
  var_dump($a);
}
<<__EntryPoint>> function main(): void {
foo(array()[]);
}
