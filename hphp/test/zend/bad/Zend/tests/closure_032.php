<?php

function test(closure $a) {
        $a(23);
}


$c = function($param) { print_r(debug_backtrace()); debug_print_backtrace(); };

$c(23);
test($c);
?>