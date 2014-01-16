<?php
/* Prototype  : array scandir(string $dir [, int $sorting_order [, resource $context]])
 * Description: List files & directories inside the specified path 
 * Source code: ext/standard/dir.c
 */

/*
 * Test basic functionality of scandir()
 */

echo "*** Testing scandir() : basic functionality ***\n";

// include file.inc for create_files function
include (dirname(__FILE__) . '/../file/file.inc');

// set up directory
$directory = dirname(__FILE__) . '/scandir_basic';
mkdir($directory);
create_files($directory, 3);

echo "\n-- scandir() with mandatory arguments --\n";
var_dump(scandir($directory));

echo "\n-- scandir() with all arguments --\n";
$sorting_order = SCANDIR_SORT_DESCENDING;
$context = stream_context_create();
var_dump(scandir($directory, $sorting_order, $context));

delete_files($directory, 3);
?>
===DONE===
<?php
$directory = dirname(__FILE__) . '/scandir_basic';
rmdir($directory);
?>