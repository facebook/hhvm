<?php

<<__EntryPoint>>
function main_empty_expr_no_var() {
error_reporting(-1);

$x = 10;
$y = 20;
var_dump(empty($x + $y));
}
