<?php

function gen() {
    var_dump(yield "yield foo");
    var_dump(yield "yield bar");
}

$gen = gen();
var_dump($gen->current());
$gen->send("send bar");
var_dump($gen->current());
$gen->send("send foo");

?>