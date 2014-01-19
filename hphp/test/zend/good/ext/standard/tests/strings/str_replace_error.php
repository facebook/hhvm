<?php
/* 
  Prototype: mixed str_replace(mixed $search, mixed $replace, 
                               mixed $subject [, int &$count]);
  Description: Replace all occurrences of the search string with 
               the replacement string
*/


echo "\n*** Testing str_replace error conditions ***";
/* Invalid arguments */
var_dump( str_replace() );
var_dump( str_replace("") );
var_dump( str_replace(NULL) );
var_dump( str_replace(1, 2) );
var_dump( str_replace(1,2,3,$var,5) );

?>
===DONE===