<?hh
function foo() :AsyncGenerator<mixed,mixed,void>{
  yield 1;
  yield -100 => 2;
  yield 3;
}

function main() :mixed{
  foreach (foo() as $k => $v) {
    var_dump($k, $v);
  }
}

<<__EntryPoint>>
function main_2231() :mixed{
main();
}
