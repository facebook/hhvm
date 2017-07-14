<?php
function test() : int {
    $d = 1.5;
    try {
        return $d;
    } finally {
        var_dump($d);
    }
}
var_dump(test());
?>
