<?php

function gen() {
    yield 'thisThrows';
    yield 'notReached';
}

$gen = gen();
var_dump($gen->throw(new RuntimeException('test')));

?>