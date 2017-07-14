<?php

function gen() {
    yield;
    return;
}

function gen2() {
    yield;
    return null;
}

function gen3() {
	return;
    yield;
}

function gen4() {
	return;
    yield;
}

var_dump(gen());

var_dump(gen2());

var_dump(gen3());

var_dump(gen4());

?>
