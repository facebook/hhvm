<?php

function gen() {
    $a = 1;
    yield $a;
}

@eval('abc');

$values = gen();
$values->next();

?>
===DONE===
