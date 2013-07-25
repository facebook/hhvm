<?php

function reverse(array $array) {
    end($array);
    while (null !== $key = key($array)) {
        yield $key => current($array);
        prev($array);
    }
}

$array = [
    'foo' => 'bar',
    'bar' => 'foo',
];

foreach (reverse($array) as $key => $value) {
    echo $key, ' => ', $value, "\n";
}

?>