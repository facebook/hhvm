<?php

function test($a) {
 var_dump(func_get_args());
}
 $a = 'Test';
 $a(1);
 $a(1, 2);
