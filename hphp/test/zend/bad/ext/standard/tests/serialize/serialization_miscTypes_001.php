<?php
ini_set('serialize_precision', 17);
 
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


echo "--- Testing Various Types ---\n";

/* unset variable */
$unset_var = 10;
unset($unset_var);
/* array declaration */
$arr_var = array(0, 1, -2, 3.333333, "a", array(), array(NULL));

$Variation_arr = array( 
   /* Integers */
   2147483647,
   -2147483647,
   2147483648,
   -2147483648,

   0xFF00123,  // hex integers
   -0xFF00123,
   0x7FFFFFFF,
   -0x7FFFFFFF, 
   0x80000000,
   -0x80000000,

   01234567,  // octal integers
   -01234567,

   /* arrays */
   array(),  // zero elements
   array(1, 2, 3, 12345666, -2344),
   array(0, 1, 2, 3.333, -4, -5.555, TRUE, FALSE, NULL, "", '', " ", 
         array(), array(1,2,array()), "string", new stdclass
        ),
   &$arr_var,  // Reference to an array
 
  /* nulls */
   NULL,
   null,

  /* strings */
   "",
   '',
   " ",
   ' ',
   "a",
   "string",
   'string',
   "hello\0",
   'hello\0',
   "123",
   '123',
   '\t',
   "\t",

   /* booleans */
   TRUE,
   true,
   FALSE,
   false,

   /* Mixed types */
   @TRUE123,
   "123string",
   "string123",
   "NULLstring",

   /* unset/undefined  vars */
   @$unset_var,
   @$undefined_var,
);

/* Loop through to test each element in the above array */
for( $i = 0; $i < count($Variation_arr); $i++ ) {

  echo "\n-- Iteration $i --\n";
  echo "after serialization => "; 
  $serialize_data = serialize($Variation_arr[$i]);
  var_dump( $serialize_data );

  echo "after unserialization => "; 
  $unserialize_data = unserialize($serialize_data);
  var_dump( $unserialize_data );
}

echo "\nDone";
?>