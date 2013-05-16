<?php
/* Prototype  : mixed mb_substitute_character([mixed substchar])
 * Description: Sets the current substitute_character or returns the current substitute_character 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

echo "*** Testing mb_substitute_character() : basic functionality ***\n";


// Initialise all required variables
var_dump( mb_substitute_character() );
var_dump( mb_substitute_character(66) );
var_dump( mb_substitute_character() );
var_dump( mb_substitute_character(1234) );
var_dump( mb_substitute_character() );
var_dump( mb_substitute_character("none") );
var_dump( mb_substitute_character() );
var_dump( mb_substitute_character("b") );

?>
===DONE===