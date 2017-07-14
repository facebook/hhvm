<?php
function &test() {
    $a = ["ok"];
    try {
        return $a[0];
    } finally {
        $a[""] = 42;
    }
}
var_dump(test());
?>
