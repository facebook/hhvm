<?php
/* Prototype  : string ezc_realpath(string path)
 * Description: Return the resolved path
 * Source code: ext/standard/file.c
 * Alias to functions:
 */

echo "*** Testing ezc_realpath() : variation ***\n";

$paths = array('c:\\',
               'c:',
               'c' ,
               '\\' ,
               '/',
               'c:temp',
               'c:\\/',
               '/tmp/',
               '/tmp/\\',
               '\\tmp',
               '\\tmp\\');

foreach($paths as $path) {
      echo "\n--$path--\n";
      var_dump( ezc_realpath($path) );
};

?>
===DONE===
