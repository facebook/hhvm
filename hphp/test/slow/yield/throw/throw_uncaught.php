<?php

function gen() {
  yield 'thisThrows';
  yield 'notReached';
}

$gen = gen();
$gen->next();
var_dump($gen->throw(new RuntimeException('test')));
