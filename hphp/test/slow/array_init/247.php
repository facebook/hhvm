<?php

function test($x, $y) {
$a = array($x, $y);
$a[] = 3;
return $a;
}
var_dump(test(1,2));
