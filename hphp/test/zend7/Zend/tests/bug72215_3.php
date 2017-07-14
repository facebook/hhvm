<?php
function &test() {
    try {
        return $a;
    } finally {
        $a = 2;
    }
}
var_dump(test());
?>
