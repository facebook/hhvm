<?hh

function foo() :AsyncGenerator<mixed,mixed,void>{
  yield 1.0 => "hello";
  yield vec[1,2,3] => "world";
  yield new stdClass => "foobar";
}

function main() :mixed{
  foreach (foo() as $k => $v) {
    var_dump($k, $v);
  }
}

<<__EntryPoint>>
function main_2233() :mixed{
main();
}
