<?php
/* Prototype  : resource gzopen(string filename, string mode [, int use_include_path])
 * Description: Open a .gz-file and return a .gz-file pointer 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

echo "*** Testing gzopen() : basic functionality ***\n";


// Initialise all required variables
$filename = dirname(__FILE__)."/004.txt.gz";
$mode = 'r';
$use_include_path = false;

// Calling gzopen() with all possible arguments
$h = gzopen($filename, $mode, $use_include_path);
gzpassthru($h);
gzclose($h);

// Calling gzopen() with mandatory arguments
$h = gzopen($filename, $mode);
gzpassthru($h);
gzclose($h);

?>
===DONE===