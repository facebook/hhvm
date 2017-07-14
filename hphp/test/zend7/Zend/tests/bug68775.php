<?php

function a($x) {
    var_dump($x);
}

function gen() {
     a(yield);
}

$g = gen();
$g->send(1);

?>
