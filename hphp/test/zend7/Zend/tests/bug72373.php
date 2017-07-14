<?php

function foo() : Generator {
    yield 1;
    yield 2;
    yield 3;
}

foreach (foo() as $bar) {
    echo $bar . "\n";
}

?>
