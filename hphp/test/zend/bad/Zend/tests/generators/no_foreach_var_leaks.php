<?php

function gen(array $array) {
	foreach ($array as $value) {
		yield $value;
	}
}

$gen = gen(['Foo', 'Bar']);
var_dump($gen->current());

// generator is closed here, without running SWITCH_FREE

?>