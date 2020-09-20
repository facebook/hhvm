<?hh

function foo($results) {
  yield 0;
  foreach ($results as $result) {
    $result->foo = 1;
  }
  var_dump($results);
  yield 1;
}
function bar() {
  foreach (foo(varray[]) as $r) {
    var_dump($r);
  }
}

<<__EntryPoint>>
function main_2181() {
bar();
}
