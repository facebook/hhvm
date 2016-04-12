<?php

function f() {
  $a = (yield 1);
  $b = yield 2;
  list($c, $d) = (yield 3);
  list($e, $f) = yield 4;
}

echo "OK\n";
