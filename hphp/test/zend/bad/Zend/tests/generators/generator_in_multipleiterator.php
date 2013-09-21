<?php

function gen1() {
    yield 'a';
    yield 'aa';
}

function gen2() {
    yield 'b';
    yield 'bb';
}

$it = new MultipleIterator;
$it->attachIterator(gen1());
$it->attachIterator(gen2());

foreach ($it as $values) {
    var_dump($values);
}

?>