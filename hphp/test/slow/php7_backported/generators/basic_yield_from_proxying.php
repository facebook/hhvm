<?hh
function from() {
  yield "from" => 1;
  yield 2;
}
function gen() {
  yield "gen" => 0;
  yield from from();
  yield 3;
}

/* foreach API */
<<__EntryPoint>>
function main_basic_yield_from_proxying() {
  foreach (gen() as $k => $v) {
    var_dump($k, $v);
  }
  /* iterator API */
  for ($gen = gen(), $gen->next(); $gen->valid(); $gen->next()) {
    var_dump($gen->key(), $gen->current());
  }
}
