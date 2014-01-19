<?php

function foo() {
  $a = &$b;
  $b = 1;
  yield $a;
  $a = 3;
  $b = 2;
  yield $a;
}
foreach (foo() as $x) var_dump($x);
