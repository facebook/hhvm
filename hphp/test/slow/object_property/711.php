<?php

function test($x, $v) {
 var_dump($x->$v += 1);
 }
test(true, "");
test(true, "\0foo");
test(false, "");
