<?php

function gen($x) {
    switch ($x."y") {
        default:
            yield;
    }
}

$gen = gen("x");
$gen->rewind();

?>
===DONE===
