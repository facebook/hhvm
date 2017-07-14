<?php

function gen1() {
    echo "gen1\n";
    yield 1;
}

function gen2() {
    try {
        echo "try\n";
        yield from gen1();
    } finally {
        echo "finally\n";
        yield from gen1();
    }
}

try {
    $gen = gen2();
    $gen->rewind();
    unset($gen);
} catch (Error $e) {
    echo $e, "\n";
}

?>
