<?php

function test($x, $v) {
 var_dump($x->$v += 1);
 }

<<__EntryPoint>>
function main_712() {
test(false, "\0foo");
}
