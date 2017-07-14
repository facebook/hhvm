<?php

function test_calls($func) {
    $i = 1;

    array_map($func, [['i' => new stdClass]]);
    var_dump($i);

    $func(['i' => new stdClass]);
    var_dump($i);

    call_user_func($func, ['i' => new stdClass]);
    var_dump($i);
}
test_calls('extract');

?>
