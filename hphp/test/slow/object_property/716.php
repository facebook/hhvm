<?php

function test($x, $v) {
 unset($x->$v);
 var_dump($x);
 }
test(new stdclass, "\0foo");
