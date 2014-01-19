<?php

function test($x, $v) {
 var_dump($x->$v);
 }
test(false, "");
test(false, "\0foo");
test(true, "");
test(true, "\0foo");
test(new stdclass, "");
