<?php

function gen() {
    try {
        echo "before return\n";
        return;
        echo "after return\n";
    } finally {
        echo "before yield\n";
        yield "yielded value";
        echo "after yield\n";
    }

    echo "after finally\n";
}

$gen = gen();
var_dump($gen->current());
$gen->next();

?>