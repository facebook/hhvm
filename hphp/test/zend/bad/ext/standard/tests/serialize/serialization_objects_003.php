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

echo "\n--- Testing Abstract Class ---\n";
// abstract class
abstract class Name 
{
  public function Name() {
    $this->a = 10;
    $this->b = 12.222;
    $this->c = "string";
  }
  abstract protected function getClassName();
  public function printClassName () {
    return $this->getClassName();
  } 
}
// implement abstract class
class extendName extends Name 
{
  var $a, $b, $c;

  protected function getClassName() {
    return "extendName";
  }
}

$obj_extendName = new extendName();
$serialize_data = serialize($obj_extendName);
var_dump( $serialize_data );
$unserialize_data = unserialize($serialize_data);
var_dump( $unserialize_data );

$serialize_data = serialize($obj_extendName->printClassName());
var_dump( $serialize_data );
$unserialize_data = unserialize($serialize_data);
var_dump( $unserialize_data );

echo "\nDone";
?>