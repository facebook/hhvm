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

echo "\n--- Testing objects ---\n";

class members 
{
  private $var_private = 10;
  protected $var_protected = "string";
  public $var_public = array(-100.123, "string", TRUE);
}

$members_obj = new members();
var_dump( $members_obj );
$serialize_data = serialize( $members_obj );
var_dump( $serialize_data );
$members_obj = unserialize( $serialize_data );
var_dump( $members_obj );

echo "\n--- testing reference to an obj ---\n";
$ref_members_obj = &$members_obj;
$serialize_data = serialize( $ref_members_obj );
var_dump( $serialize_data );
$ref_members_obj = unserialize( $serialize_data );
var_dump( $ref_members_obj );

echo "\nDone";
?>