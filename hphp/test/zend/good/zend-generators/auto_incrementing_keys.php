<?php

function gen() {
    yield 'foo';
    yield 'bar';
    yield 5 => 'rab';
    yield 'oof';
}

foreach (gen() as $k => $v) {
    echo $k, ' => ', $v, "\n";
}

?>