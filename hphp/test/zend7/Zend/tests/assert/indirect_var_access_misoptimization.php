<?php

function test() {
    $i = 0;
    assert('$i = new stdClass');
    $i += 1;
    var_dump($i);
}
test();

?>
