<?php

function test($x, $v) {
 var_dump($x->$v = 1);
 }
test(false, "\0foo");
