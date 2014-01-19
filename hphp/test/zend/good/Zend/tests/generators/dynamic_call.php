<?php

function gen($foo, $bar) {
    yield $foo;
    yield $bar;
}

$gen = call_user_func('gen', 'bar', 'foo');
foreach ($gen as $value) {
    var_dump($value);
}

?>