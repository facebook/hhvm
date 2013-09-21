<?php
/*  Prototype: bool mkdir ( string $pathname [, int $mode [, bool $recursive [, resource $context]]] );
    Description: Makes directory

    Prototype: bool rmdir ( string $dirname [, resource $context] );
    Description: Removes directory
*/

echo "*** Testing mkdir(): error conditions ***\n";
var_dump( mkdir() );  // args < expected
var_dump( mkdir(1, 2, 3, 4, 5) );  // args > expected
var_dump( mkdir("testdir", 0777, false, $context, "test") );  // args > expected

echo "\n*** Testing rmdir(): error conditions ***\n";
var_dump( rmdir() );  // args < expected
var_dump( rmdir(1, 2, 3) );  // args > expected
var_dump( rmdir("testdir", $context, "test") );  // args > expected

echo "\n*** Testing rmdir() on non-existent directory ***\n";
var_dump( rmdir("temp") );

echo "Done\n";
?>