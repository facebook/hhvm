<?php

/* Miscellaneous input */
echo "\n*** Testing addcslashes() with miscellaneous input arguments ***\n";
var_dump( addcslashes("", "") );
var_dump( addcslashes("", "burp") );
var_dump( addcslashes("kaboemkara!", "") );
var_dump( addcslashes("foobarbaz", 'bar') );
var_dump( addcslashes('foo[ ]', 'A..z') );
var_dump( @addcslashes("zoo['.']", 'z..A') );
var_dump( addcslashes('abcdefghijklmnopqrstuvwxyz', "a\145..\160z") );
var_dump( addcslashes( 123, 123 ) );
var_dump( addcslashes( 123, NULL) );
var_dump( addcslashes( NULL, 123) );
var_dump( addcslashes( 0, 0 ) );
var_dump( addcslashes( "\0" , 0 ) );
var_dump( addcslashes( NULL, NULL) );
var_dump( addcslashes( -1.234578, 3 ) );
var_dump( addcslashes( " ", " ") );
var_dump( addcslashes( "string\x00with\x00NULL", "\0") );

echo "Done\n"; 

?>