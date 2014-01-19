<?php

function test($x, $v) {
 var_dump($x->$v++);
 }
test(false, "\0foo");
