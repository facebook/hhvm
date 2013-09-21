<?php

function gen() {
    try {
        echo "before yield\n";
        yield;
        echo "after yield\n";
    } finally {
        echo "before yield in finally\n";
        yield;
        echo "after yield in finally\n";
    }

    echo "after finally\n";
}

$gen = gen();
$gen->rewind();
unset($gen);

?>