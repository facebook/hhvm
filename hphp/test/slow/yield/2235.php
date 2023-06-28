<?hh

function foo() :AsyncGenerator<mixed,mixed,void>{
  yield 1;
  yield "7" => 2;
  yield 3;
}

function main() :mixed{
  foreach(foo() as $k => $v) {
    var_dump($k, $v);
  }
}

<<__EntryPoint>>
function main_2235() :mixed{
main();
}
