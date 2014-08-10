<?php
/* Prototype  : proto bool trait_exists(string traitname [, bool autoload])
 * Description: Checks if the trait exists 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */

echo "*** Testing trait_exists() : basic functionality ***\n";

function __autoload($traitName) {
	echo "In __autoload($traitName)\n";
}

trait MyTrait {}

echo "Calling trait_exists() on non-existent trait with autoload explicitly enabled:\n";
var_dump( trait_exists('C', true) );
echo "\nCalling trait_exists() on existing trait with autoload explicitly enabled:\n";
var_dump( trait_exists('MyTrait', true) );

echo "\nCalling trait_exists() on non-existent trait with autoload explicitly enabled:\n";
var_dump( trait_exists('D', false) );
echo "\nCalling trait_exists() on existing trait with autoload explicitly disabled:\n";
var_dump( trait_exists('MyTrait', false) );

echo "\nCalling trait_exists() on non-existent trait with autoload unspecified:\n";
var_dump( trait_exists('E') );
echo "\nCalling trait_exists() on existing trait with autoload unspecified:\n";
var_dump( trait_exists('MyTrait') );

echo "Done";
?>
