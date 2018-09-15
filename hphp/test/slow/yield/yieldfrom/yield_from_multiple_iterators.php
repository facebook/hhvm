<?php

function gen() {
  yield from [1, 2, 3, 4];
  yield from [11, 12, 13];
  yield from [];
  yield from [31, 32, 33, 34, 35];
}


<<__EntryPoint>>
function main_yield_from_multiple_iterators() {
foreach(gen() as $val) {
  var_dump($val);
}
}
