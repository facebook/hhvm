<?php

function gen() {
    try {
        yield;
    } catch (RuntimeException $e) {
        echo 'Caught: ', $e, "\n\n";

        throw new LogicException('new throw');
    }
}

$gen = gen();
var_dump($gen->throw(new RuntimeException('throw')));

?>