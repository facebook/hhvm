<?php

function from($levels) {
	foreach (range(0, 2 << $levels) as $v) {
		yield $v;
		if ($v == (1 << ($levels - 1)) - 2) {
			throw new Exception();
		}
	}
}

function gen($gen, $level) {
	yield from $gen;
}

$levels = 5;

print "$levels levels\n\n";

$all = array();
$all[] = $gens[0][0] = from($levels);

for ($level = 1; $level < $levels; $level++) {
	for ($i = 0; $i < (1 << $level); $i++) {
		$all[] = $gens[$level][$i] = gen($gens[$level-1][$i >> 1], $level);
	}
}

for ($i = 0; $i < 2; $i++) { 
	try {
		foreach ($all as $gen) {
			var_dump($gen->current());
			$gen->next();
			if (!$gen->valid()) {
				break;
			}
		}
	} catch(Exception $e) {
		print "$e\n";
		unset($all[array_search($gen, $all)]);
	}
}
?>
