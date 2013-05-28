<?php

function test($x, $v) {
 var_dump($x->$v++);
 }
test(true, "");
test(true, "\0foo");
test(false, "");
