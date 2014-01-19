<?php

function using($cgen) {
  foreach ($cgen() as $x) {
    yield $x;
  }
}

function broke() {
    foreach (using(function() { yield 1; yield 2; yield 3; }) as $x) {
      var_dump($x);
    }
}
broke();

class c {
  function genclo() {
    return function() {
      yield $this;
    };
  }
}

function main() {
  $c = new c;
  $f = $c->genclo();
  foreach ($f() as $v) {
    var_dump($v);
  }
}
main();
