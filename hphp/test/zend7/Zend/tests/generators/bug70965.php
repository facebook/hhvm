<?php

function it() {
    yield from [1, 2, 3, 4, 5];
}

function bar($g) {
    yield from $g;
}

$gen = it();
$gens[] = bar($gen);
$gens[] = bar($gen);

do {
    foreach($gens as $g) {
        var_dump($g->current());
        $gen->next();
    }
} while ($gen->valid());

?>
