<?php

function foo($x, $y) {
  foo($x, $y, $x + $y, $y + $x);
}

foo(1, 2);
