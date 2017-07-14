<?php
function test() {
    $a = 1;
    try {
        return $a;
    } finally {
        $a = 2;
    }
}
var_dump(test());
?>
