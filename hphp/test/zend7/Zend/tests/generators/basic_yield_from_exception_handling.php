<?php
function from($off) {
	try {
		yield $off + 1;
	} catch (Exception $e) { print "catch in from()\n$e\n"; }
	yield $off + 2;
}

function gen() {
	try {
		yield "gen" => 0;
	} catch (Exception $e) { print "catch in gen()\n$e\n"; }
	try {
		yield from from(0);
	} catch (Exception $e) { print "catch in gen()\n$e\n"; }
	yield from from(2);
}

$i = 0;
try {
	for ($gen = gen(); $gen->valid(); $gen->throw(new Exception((string) $i++))) {
		var_dump($gen->current());
	}
} catch (Exception $e) { print "catch in {main}\n$e\n"; }

var_dump($gen->valid());

?>
