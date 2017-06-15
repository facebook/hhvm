<?php
function test1($x) {
    return $x + 1;
}

function test2($y) {
    return test1(
        $y * 2
    );
}

xdebug_start_code_coverage(XDEBUG_CC_DEAD_CODE);
$z = test2(42);
var_dump(xdebug_get_code_coverage());
