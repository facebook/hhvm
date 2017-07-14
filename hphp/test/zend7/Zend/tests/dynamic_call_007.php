<?php

function test() {
    $i = 1;
    array_map('extract', [['i' => new stdClass]]);
    $i += 1;
    var_dump($i);
}
test();

?>
