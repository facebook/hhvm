<?php

/* Testing Error Conditions */
echo "*** Testing Error Conditions ***\n";

/* zero argument */
var_dump( fprintf() );

/* scalar argument */
var_dump( fprintf(3) );

/* NULL argument */
var_dump( fprintf(NULL) );

echo "Done\n";
?>