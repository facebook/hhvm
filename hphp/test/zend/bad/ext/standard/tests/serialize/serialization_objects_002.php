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

echo "\n--- Testing Variations in objects ---\n";

class members 
{
  private $var_private = 10;
  protected $var_protected = "string";
  public $var_public = array(-100.123, "string", TRUE);
}

class nomembers { }

class C {
  var $a, $b, $c, $d, $e, $f, $g, $h;
  function __construct() {
    $this->a = 10;
    $this->b = "string";
    $this->c = TRUE;
    $this->d = -2.34444;
    $this->e = array(1, 2.22, "string", TRUE, array(), 
                     new members(), null);
    $this->f = new nomembers();
    $this->g = $GLOBALS['file_handle'];
    $this->h = NULL;
  }
}

class D extends C {
  function __construct( $w, $x, $y, $z ) {
    $this->a = $w;
    $this->b = $x;
    $this->c = $y;
    $this->d = $z;
  }
}

$variation_obj_arr = array(
  new C(),
  new D( 1, 2, 3333, 444444 ),
  new D( .5, 0.005, -1.345, 10.005e5 ),
  new D( TRUE, true, FALSE, false ),
  new D( "a", 'a', "string", 'string' ),
  new D( array(), 
         array(1, 2.222, TRUE, FALSE, "string"), 
         array(new nomembers(), $file_handle, NULL, ""),
         array(array(1,2,3,array()))
       ),
  new D( NULL, null, "", "\0" ),
  new D( new members, new nomembers, $file_handle, NULL),
);   

/* Testing serialization on all the objects through loop */
foreach( $variation_obj_arr as $object) {

  echo "After Serialization => ";
  $serialize_data = serialize( $object );
  var_dump( $serialize_data );
 
  echo "After Unserialization => ";
  $unserialize_data = unserialize( $serialize_data );
  var_dump( $unserialize_data );
}

echo "\nDone";
?>