<?php

function gen() {
    echo "before yield\n";
    try {
        yield;
    } catch (RuntimeException $e) {
        echo $e, "\n\n";
    }

    yield 'result';
}

$gen = gen();
var_dump($gen->throw(new RuntimeException('Test')));

?>