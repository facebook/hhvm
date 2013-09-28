<?php

function test($a, $b) {
 var_dump(func_get_args());
}
 $a = 'Test';
 $a(1, 2);
 $a(1, 2, 3);
 $a(1, 2, 3, 4);
