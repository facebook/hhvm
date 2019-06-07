<?hh
function from($a = 0) {
  yield 1 + $a;
  if ($a <= 3) {
    yield from from($a + 3);
    yield from from($a + 6);
  }
  yield 2 + $a;
}
function gen() {
  yield from from();
}

<<__EntryPoint>>
function main_recursive_yield_from() {
foreach(gen() as $v) {
  var_dump($v);
}
}
