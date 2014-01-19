<?php

function foo() {
 return 42;
 }
$a = foo();
var_dump((unset)foo());
var_dump((unset)$a);
var_dump($a);
