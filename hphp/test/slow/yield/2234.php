<?hh

function foo() :AsyncGenerator<mixed,mixed,void>{
  yield PHP_INT_MAX => 1;
  yield 2;
}

function main() :mixed{
  foreach (foo() as $k => $v) {
    var_dump($k, $v);
  }
}

<<__EntryPoint>>
function main_2234() :mixed{
main();
}
