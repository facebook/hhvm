<?php
function test1() : Generator {
    yield 1;
}

function test2() : Iterator {
    yield 2;
}

function test3() : Traversable {
    yield 3;
}

var_dump(
    test1(),
    test2(),
    test3()
);

