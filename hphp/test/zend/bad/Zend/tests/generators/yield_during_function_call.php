<?php

function gen() {
	var_dump(str_repeat("x", yield));
}

$gen = gen();
$gen->send(10);

?>