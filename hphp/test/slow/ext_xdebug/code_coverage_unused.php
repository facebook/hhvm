<?php
function test1() {
    return 1;
}

function test2() {
    return 2;
}

xdebug_start_code_coverage(XDEBUG_CC_UNUSED);
$x = test1();
var_dump(xdebug_get_code_coverage());
