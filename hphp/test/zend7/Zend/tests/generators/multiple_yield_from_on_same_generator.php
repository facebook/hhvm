<?php

function gen($a = 0) {
	yield 1 + $a;
	if ($a < 1) {
		var_dump(yield from gen($a + 1));
	}
	yield 3 + $a;
	return 5 + $a;
}

function bar($gen) {
	var_dump(yield from $gen);
}

/* Twice a Generator from bar() using yield from on $gen */
$gen = gen();
$gens[] = bar($gen);
$gens[] = bar($gen);

do {
	foreach ($gens as $g) {
		var_dump($g->current());
		$g->next();
	}
} while($gens[0]->valid());
var_dump($gens[1]->valid());

?>
