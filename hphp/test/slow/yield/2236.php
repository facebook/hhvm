<?hh

function foo() :AsyncGenerator<mixed,mixed,void>{
  $a = new stdClass;
  yield $a => 1;
  yield $a => 2;
  yield $a => 3;
}

function main() :mixed{
  foreach(foo() as $k => $v) {
    var_dump($k, $v);
  }
}

<<__EntryPoint>>
function main_2236() :mixed{
main();
}
