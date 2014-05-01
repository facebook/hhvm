<?php

function fruit() {
  $a = 123;
  yield $a;
  return;
  yield ++$a;
}

foreach (fruit() as $fruit) {
  var_dump($fruit);
}

