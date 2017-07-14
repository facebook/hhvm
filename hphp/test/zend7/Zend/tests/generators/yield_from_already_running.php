<?php

function gen() {
	yield from yield;
}

($gen = gen())->send($gen);

?>
