<?php

function test($x, $v) {
 var_dump($x->$v += 1);
 }
test(new stdclass, "\0foo");
