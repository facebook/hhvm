<?php

echo "\n*** Testing error conditions ***\n";
/* Zero argument */
try { var_dump( substr_count() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* more than expected no. of args */
try { var_dump( substr_count($str, "t", 0, 15, 30) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
	
/* offset as negative value */
var_dump(substr_count($str, "t", -5));

/* offset > size of the string */
var_dump(substr_count($str, "t", 25));

/* Using offset and length to go beyond the size of the string: 
   Warning message expected, as length+offset > length of string */
var_dump( substr_count($str, "i", 5, 15) );

/* length as Null */
try { var_dump( substr_count($str, "t", "", "") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump( substr_count($str, "i", NULL, NULL) );
	
echo "Done\n";	

