<?php

function test($x, $v) {
 var_dump($x->$v++);
 }
test(new stdclass, "\0foo");
