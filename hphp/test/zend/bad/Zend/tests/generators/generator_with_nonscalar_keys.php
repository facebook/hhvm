<?php

function gen() {
    yield [1, 2, 3] => [4, 5, 6];
    yield (object) ['a' => 'b'] => (object) ['b' => 'a'];
    yield 3.14 => 2.73;
    yield false => true;
    yield true => false;
    yield null => null;
}

foreach (gen() as $k => $v) {
    var_dump($k, $v);
}

?>