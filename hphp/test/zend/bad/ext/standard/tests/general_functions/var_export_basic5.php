<?php
ini_set('serialize_precision', 17);

/* Prototype  : mixed var_export(mixed var [, bool return])
 * Description: Outputs or returns a string representation of a variable 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */


echo "*** Testing var_export() with valid arrays ***\n";
// different valid  arrays 
$valid_arrays = array(
           "array()" => array(),
           "array(NULL)" => array(NULL),
           "array(null)" => array(null),
           "array(true)" => array(true),
           "array(\"\")" => array(""),
           "array('')" => array(''),
           "array(array(), array())" => array(array(), array()),
           "array(array(1, 2), array('a', 'b'))" => array(array(1, 2), array('a', 'b')),
           "array(1 => 'One')" => array(1 => 'One'),
           "array(\"test\" => \"is_array\")" => array("test" => "is_array"),
           "array(0)" => array(0),
           "array(-1)" => array(-1),
           "array(10.5, 5.6)" => array(10.5, 5.6),
           "array(\"string\", \"test\")" => array("string", "test"),
           "array('string', 'test')" => array('string', 'test')
);

/* Loop to check for above arrays with var_export() */
echo "\n*** Output for arrays ***\n";
foreach($valid_arrays as $key => $arr) {
	echo "\n--Iteration: $key --\n";
	var_export( $arr );
	echo "\n";
	var_export( $arr, FALSE);
	echo "\n";
	var_dump( var_export( $arr, TRUE) );
	echo "\n";
}
?>
===DONE===