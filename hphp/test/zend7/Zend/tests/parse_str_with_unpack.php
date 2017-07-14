<?php

function test() {
    $i = 0;
    parse_str(...["i=41"]);
    var_dump($i + 1);
}
test();

?>
