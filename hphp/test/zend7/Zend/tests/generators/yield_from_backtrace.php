<?php
function from($off) {
	debug_print_backtrace();
	yield $off + 1;
}

function gen() {
	yield 1;
	debug_print_backtrace();
	yield 2;
	yield from from(2);
	debug_print_backtrace();
}

print "\nImplicit foreach:\n";
foreach (gen() as $v) {
	var_dump($v);
}

print "\nExplicit iterator:\n";
for ($gen = gen(); $gen->valid(); $gen->next()) {
	var_dump($gen->current());
}
?>
