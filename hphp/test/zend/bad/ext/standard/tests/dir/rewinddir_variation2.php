<?php
/* Prototype  : void rewinddir([resource $dir_handle])
 * Description: Rewind dir_handle back to the start 
 * Source code: ext/standard/dir.c
 * Alias to functions: rewind
 */

/*
 * Open and close a directory handle then call rewinddir() to test behaviour
 */

echo "*** Testing rewinddir() : usage variations ***\n";

$dir_path = dirname(__FILE__) . '/rewinddir_variation2';
mkdir($dir_path);

echo "\n-- Create the directory handle, read and close the directory --\n";
var_dump($dir_handle = opendir($dir_path));
var_dump(readdir($dir_handle));
closedir($dir_handle);

echo "\n-- Call to rewinddir() --\n";
var_dump(rewinddir($dir_handle));
?>
===DONE===
<?php error_reporting(0); ?>
<?php
$dir_path = dirname(__FILE__) . '/rewinddir_variation2';
rmdir($dir_path);
?>