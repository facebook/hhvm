<?hh

function foo($results) :AsyncGenerator<mixed,mixed,void>{
  yield 0;
  foreach ($results as $result) {
    $result->foo = 1;
  }
  var_dump($results);
  yield 1;
}
function bar() :mixed{
  foreach (foo(vec[]) as $r) {
    var_dump($r);
  }
}

<<__EntryPoint>>
function main_2181() :mixed{
bar();
}
