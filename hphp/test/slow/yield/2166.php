<?php

function get() {
 return true;
 }
if (get()) {
  function gen($i) {
    yield $i;
    yield $i + 1;
  }
}
 else {
  function gen($i) {
    yield $i + 1;
    yield $i + 2;
  }
}
foreach (gen(3) as $x) {
 var_dump($x);
 }
