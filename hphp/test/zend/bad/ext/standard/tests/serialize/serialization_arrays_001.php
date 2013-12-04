<?php
ini_set('serialize_precision', 100);
 
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

echo "\n--- Testing Circular reference of an array ---\n";

echo "-- Normal array --\n";
$arr_circ = array(0, 1, -2, 3.333333, "a", array(), &$arr_circ);
$serialize_data = serialize($arr_circ);
var_dump( $serialize_data );
$arr_circ = unserialize($serialize_data);
var_dump( $arr_circ );

echo "\n-- Associative array --\n";
$arr_asso = array("a" => "test");
$arr_asso[ "b" ] = &$arr_asso[ "a" ];
var_dump($arr_asso);
$serialize_data = serialize($arr_asso);
var_dump($serialize_data);
$arr_asso = unserialize($serialize_data);
var_dump($arr_asso);

echo "\nDone";
?>