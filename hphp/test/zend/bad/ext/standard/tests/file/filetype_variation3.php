<?php
/*
Prototype: string filetype ( string $filename );
Description: Returns the type of the file. Possible values are fifo, char,
             dir, block, link, file, and unknown. 
*/

echo "-- Checking for block --\n";
print( filetype("/dev/ram0") )."\n";
?>
===DONE===