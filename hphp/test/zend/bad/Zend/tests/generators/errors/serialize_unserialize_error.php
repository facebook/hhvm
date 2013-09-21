<?php

function gen() { yield; }

$gen = gen();

try {
    serialize($gen);
} catch (Exception $e) {
    echo $e, "\n\n";
}

try {
    var_dump(unserialize('O:9:"Generator":0:{}'));
} catch (Exception $e) {
    echo $e, "\n\n";
}

try {
    var_dump(unserialize('C:9:"Generator":0:{}'));
} catch (Exception $e) {
    echo $e;
}

?>