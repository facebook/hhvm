<?php

function test($x, $v) {
 var_dump($x->$v++);
 }

<<__EntryPoint>>
function main_708() {
test(false, "\0foo");
}
