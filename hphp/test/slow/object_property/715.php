<?php

function test($x, $v) {
 unset($x->$v);
 var_dump($x);
 }
test(false, "");
test(false, "\0foo");
test(true, "");
test(true, "\0foo");
test(new stdclass, "");
