<?php

function gen() {
    try {
        yield;
    } finally {
        var_dump($_GET);
    }
}

$gen = gen();
$gen->rewind();

set_error_handler(function() use($gen) {});

?>