<?php

const STRING_VAL = "test";

function int_val(int $a = STRING_VAL): int {
    return $a;
}

var_dump(int_val());

?>
