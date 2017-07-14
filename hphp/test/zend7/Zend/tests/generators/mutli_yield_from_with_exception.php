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
	$g->current(); // init.
}

do {
	foreach ($gens as $i => $g) {
		print "Generator $i\n";
		var_dump($g->current());
		$g->next();
	}
} while($gens[0]->valid());
?>
