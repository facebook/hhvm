<?php

if (false) {
  function gen1() {
    yield 1;
  }
}
if (true) {
  function gen1() {
    yield 2;
  }
  function gen2() {
    yield 3;
  }
}
if (false) {
  function gen2() {
    yield 4;
  }
}

foreach (gen1() as $x) {
  var_dump($x);
}
foreach (gen2() as $x) {
  var_dump($x);
}
