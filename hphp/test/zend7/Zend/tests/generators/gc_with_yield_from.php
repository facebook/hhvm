<?php

function root() {
	global $gens; // create cyclic reference to root
	try {
		yield 1;
	} finally {
		var_dump($gens);
	}
}

function gen($x) {
	global $gens;
	yield from $gens[] = $x ? gen(--$x) : root();
}

$gen = $gens[] = gen(2);
var_dump($gen->current());
unset($gen, $gens);
print "collect\n";
gc_collect_cycles();
print "end\n";

?>
