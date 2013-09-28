<?php

echo "\n*** Testing error conditions ***\n";
/* Zero argument */
var_dump( substr_count() );

/* more than expected no. of args */
var_dump( substr_count($str, "t", 0, 15, 30) );
	
/* offset as negative value */
var_dump(substr_count($str, "t", -5));

/* offset > size of the string */
var_dump(substr_count($str, "t", 25));

/* Using offset and length to go beyond the size of the string: 
   Warning message expected, as length+offset > length of string */
var_dump( substr_count($str, "i", 5, 15) );

/* length as Null */
var_dump( substr_count($str, "t", "", "") );
var_dump( substr_count($str, "i", NULL, NULL) );
	
echo "Done\n";	

?>