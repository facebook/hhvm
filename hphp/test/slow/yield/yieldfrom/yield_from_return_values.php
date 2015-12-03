<?php

function gen($x) {
    yield $x;
    return $x * 5;
}

function yf() {
    $x = yield from gen(1);
    echo "x is $x\n";
    $y = yield from gen(2);
    echo "y is $y\n";
    return yield from gen(42);
}

$g = yf();
foreach($g as $val) { var_dump($val); }
echo "====================\n";
var_dump($g->getReturn());
