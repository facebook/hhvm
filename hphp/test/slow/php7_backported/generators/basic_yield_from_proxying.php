<?php
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
foreach (gen() as $k => $v) {
  var_dump($k, $v);
}
/* iterator API */
for ($gen = gen(); $gen->valid(); $gen->next()) {
  var_dump($gen->key(), $gen->current());
}
