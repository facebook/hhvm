<?php

function test($x, $v) {
 var_dump(isset($x->$v));
 }
test(false, "");
test(false, "\0foo");
test(true, "");
test(true, "\0foo");
test(new stdclass, "");
test(new stdclass, "\0foo");
