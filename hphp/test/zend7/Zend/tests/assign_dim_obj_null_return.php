<?php

function test() {
    $array = [PHP_INT_MAX => 42];
    $true = true;

    var_dump($array[] = 123);
    var_dump($array[[]] = 123);
    var_dump($array[new stdClass] = 123);
    var_dump($true[123] = 456);

    var_dump($array[] += 123);
    var_dump($array[[]] += 123);
    var_dump($array[new stdClass] += 123);
    var_dump($true[123] += 456);

    var_dump($true->foo = 123);
    var_dump($true->foo += 123);
}

test();

?>
