<?php

var_dump(define());
var_dump(define("TRUE"));
var_dump(define("TRUE", 1));
var_dump(define("TRUE", 1, array(1)));

var_dump(define(array(1,2,3,4,5), 1));
var_dump(define(" ", 1));
var_dump(define("[[[", 2));
var_dump(define("test const", 3));
var_dump(define("test const", 3));
var_dump(define("test", array(1)));
var_dump(define("test1", new stdclass));

var_dump(constant(" "));
var_dump(constant("[[["));
var_dump(constant("test const"));

echo "Done\n";
?>