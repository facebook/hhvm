<?hh
function from1234($x) :mixed{
  return $x;
}
function bar() :AsyncGenerator<mixed,mixed,void>{
  yield 24;
}
function foo() :AsyncGenerator<mixed,mixed,void>{
  yield from1234(42);
  yield from(bar());
}
<<__EntryPoint>> function main(): void {
foreach (foo() as $value) {
  var_dump($value);
}
}
