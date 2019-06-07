<?hh
function from($i) {
  yield $i;
}
function gen($i = 0) {
  if ($i < 1000) {
    yield from gen(++$i);
  } else {
    yield $i;
    yield from from(++$i);
  }
}

<<__EntryPoint>>
function main_yield_from_deep_recursion() {
ini_set("memory_limit", "60G");
foreach (gen() as $v) {
  var_dump($v);
}
}
