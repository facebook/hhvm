<?php

function test(iterable $iterable) {
    var_dump($iterable);
}

function gen() {
    yield 1;
    yield 2;
    yield 3;
};

test([1, 2, 3]);
test(gen());
test(new ArrayIterator([1, 2, 3]));

try {
    test(1);
} catch (Throwable $e) {
    echo $e->getMessage();
}

