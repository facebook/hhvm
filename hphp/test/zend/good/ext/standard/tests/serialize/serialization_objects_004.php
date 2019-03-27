<?php
/* Prototype  : proto string serialize(mixed variable)
 * Description: Returns a string representation of variable (which can later be unserialized)
 * Source code: ext/standard/var.c
 * Alias to functions:
 */
/* Prototype  : proto mixed unserialize(string variable_representation)
 * Description: Takes a string representation of variable and recreates it
 * Source code: ext/standard/var.c
 * Alias to functions:
 */

$x = new stdClass;
var_dump(serialize(array($x, $x)));

$x = 1;
var_dump(serialize(array($x, $x)));

$x = "a";
var_dump(serialize(array($x, $x)));

$x = true;
var_dump(serialize(array($x, $x)));

$x = null;
var_dump(serialize(array($x, $x)));

$x = array();
var_dump(serialize(array($x, $x)));

echo "Done";
