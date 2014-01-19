<?php

echo "\n*** Testing error conditions ***\n";
/* zero argument */
var_dump( addcslashes() );

/* unexpected arguments */
var_dump( addcslashes("foo[]") );
var_dump( addcslashes('foo[]', "o", "foo") );

echo "Done\n"; 

?>