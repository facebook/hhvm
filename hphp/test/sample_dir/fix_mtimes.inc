<?php
/**
 * require this in your tests if you want to make sure the mtimes and iterator 
 * order of files in this directory is consistent
 */

$dir = dirname(__FILE__).'/';
$base = 1234567890;
touch($dir, $base);
touch($dir.'dir', $base + 1);
touch($dir.'empty', $base + 2);
touch($dir.'file', $base + 3);
touch($dir.'symlink', $base + 4);
touch($dir.'fix_mtimes.inc', $base + 5);
