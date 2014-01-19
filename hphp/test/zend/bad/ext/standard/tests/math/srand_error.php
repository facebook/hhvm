<?php
/* Prototype  : void srand  ([ int $seed  ] )
 * Description: Seed the random number generator.
 * Source code: ext/standard/rand.c
 */
 
/*
 * Pass incorrect number of arguments to srand() to test behaviour
 */
 
echo "*** Testing srand() : error conditions ***\n";

var_dump(srand(500, true));
var_dump(srand("fivehundred"));
var_dump(srand("500ABC"));
?>
===Done===