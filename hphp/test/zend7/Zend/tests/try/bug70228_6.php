<?php
function test($x) {
    foreach ($x as $v) {
        try {
            return str_repeat("a", 2);
        } finally {
            return 42;
        }
    }
}

var_dump(test([1]));
?>
