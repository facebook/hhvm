<?php

function f() {
 yield func_num_args();
 yield func_get_arg(1);
 }

<<__EntryPoint>>
function main_2151() {
foreach (f(1, 2, 3) as $v) {
 var_dump($v);
 }
}
