<?php

function f($a) {
 $a[] = $a;
 var_dump($a);
 }

<<__EntryPoint>>
function main_260() {
f(false);
}
