<?php
/*
Prototype: string filetype ( string $filename );
Description: Returns the type of the file. Possible values are fifo, char,
             dir, block, link, file, and unknown. 
*/

echo "-- Checking for char --\n";
print( filetype("/dev/console") )."\n";
?>
===DONE===