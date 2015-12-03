<?php
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
foreach(gen() as $v) {
  var_dump($v);
}
