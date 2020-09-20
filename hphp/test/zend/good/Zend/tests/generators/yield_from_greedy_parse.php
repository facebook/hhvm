<?hh
function from1234($x) {
  return $x;
}
function bar() {
  yield 24;
}
function foo() {
  yield from1234(42);
  yield from(bar());
}
<<__EntryPoint>> function main(): void {
foreach (foo() as $value) {
  var_dump($value);
}
}
