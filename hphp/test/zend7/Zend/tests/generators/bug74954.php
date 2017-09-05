<?php

function from() {
        yield 1;
        throw new Exception();
}

function gen($gen) {
        try {
                var_dump(yield from $gen);
        } catch (Exception $e) { print "Caught exception!\n$e\n"; }
}

$gen = from();
$gens[] = gen($gen);
$gens[] = gen($gen);

foreach ($gens as $g) {
        $g->current();
}

do {
        foreach ($gens as $i => $g) {
                $g->next();
        }
} while($gens[0]->valid());

?>
