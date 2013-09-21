<?php

function recv() {
    while (true) {
        var_dump(yield);
    }
}

$receiver = recv();
var_dump($receiver->current());
$receiver->send(1);
var_dump($receiver->current());
$receiver->send(2);
var_dump($receiver->current());
$receiver->send(3);

?>