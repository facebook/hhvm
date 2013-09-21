<?php

function reverseEchoGenerator() {
    $data = yield;
    while (true) {
        $data = (yield strrev($data));
    }
}

$gen = reverseEchoGenerator();
var_dump($gen->send('foo'));
var_dump($gen->send('bar'));

?>