<?php

function gen1() {
  yield 1 => 2;
  yield 2 => 3;
}

function gen2() {
  if (false) {
    yield 1 => 2;
  }
}


<<__EntryPoint>>
function main_auto_priming() {
var_dump(gen1()->valid());
var_dump(gen1()->current());
var_dump(gen1()->key());
var_dump(gen1()->next());

var_dump(gen2()->valid());
var_dump(gen2()->current());
var_dump(gen2()->key());
try {
  var_dump(gen2()->next());
} catch (Exception $e) {
  var_dump($e->getMessage());
}
}
