<?php
function from() {
  yield 0;
  yield from []; // must not yield anything
  yield from [1,2];
}
function gen() {
  yield from from();
}
foreach(gen() as $v) {
  var_dump($v);
}
