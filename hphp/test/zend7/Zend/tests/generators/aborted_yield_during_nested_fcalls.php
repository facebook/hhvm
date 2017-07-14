<?php

function func() {}

function gen($x) {
    func(func($x, $x, func($x, yield)));
}

$gen = gen("x");
$gen->rewind();

?>
===DONE===
