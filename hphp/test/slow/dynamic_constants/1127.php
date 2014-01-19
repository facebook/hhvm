<?php

function foo($a) {
 return $a + 10;
}
 define('TEST', foo(10));
 var_dump(TEST);
