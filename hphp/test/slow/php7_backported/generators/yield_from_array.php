<?php
function from() {
  yield 0;
  yield from []; // must not yield anything
  yield from [1,2];
}
function gen() {
  yield from from();
}

<<__EntryPoint>>
function main_yield_from_array() {
foreach(gen() as $v) {
  var_dump($v);
}
}
