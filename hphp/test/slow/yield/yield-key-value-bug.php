<?hh
function foo() :AsyncGenerator<mixed,mixed,void>{
  yield 1 => 2;
  yield 'a' => 'b';
}
function bar() :AsyncGenerator<mixed,mixed,void>{
  foreach (foo() as $k => $v) {
    yield $k => $v;
  }
}
function main() :mixed{
  foreach (bar() as $k => $v) {
    echo "$k $v\n";
  }
}


<<__EntryPoint>>
function main_yield_key_value_bug() :mixed{
main();
}
