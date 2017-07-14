<?php

function gen() {
    yield yield;
}
$gen = gen();
$gen->send($gen);

?>
===DONE===
