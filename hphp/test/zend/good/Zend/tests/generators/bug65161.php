<?php

function autoload() {
    foo();
}
spl_autoload_register('autoload');

function testGenerator() {
    new SyntaxError('param');
    yield;
}

foreach (testGenerator() as $i);

?>