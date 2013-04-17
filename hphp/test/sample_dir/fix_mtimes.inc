<?php
/**
 * require this in your tests if you want to make sure the mtimes and iterator 
 * order of files in this directory is consistent
 */

$dir = dirname(__FILE__).'/';
$base = 1234567890;
touch($dir, $base++);
touch($dir.'dir', $base++);
touch($dir.'empty', $base++);
touch($dir.'file', $base++);
touch($dir.'symlink', $base++);
touch($dir.'fix_mtimes.inc', $base++);

// Tests that include this like to work out of the 'hphp' dir
chdir($dir.'../..');
