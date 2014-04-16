<?php

var_dump(file_exists(          __FILE__));
var_dump(file_exists('file://'.__FILE__)); echo "\n";

var_dump(is_file(          __FILE__));
var_dump(is_file('file://'.__FILE__)); echo "\n";

var_dump(is_dir(          __DIR__));
var_dump(is_dir('file://'.__DIR__)); echo "\n";

var_dump(is_readable(          __FILE__));
var_dump(is_readable('file://'.__FILE__)); echo "\n";

var_dump(is_writable(          __FILE__) ===
         is_writable('file://'.__FILE__));

var_dump(is_executable(          __FILE__) ===
         is_executable('file://'.__FILE__));

var_dump(fileperms(          __FILE__) ===
         fileperms('file://'.__FILE__));

var_dump(fileinode(          __FILE__) ===
         fileinode('file://'.__FILE__));

var_dump(fileowner(          __FILE__) ===
         fileowner('file://'.__FILE__));

var_dump(filegroup(          __FILE__) ===
         filegroup('file://'.__FILE__));

var_dump(fileatime(          __FILE__) ===
         fileatime('file://'.__FILE__));

var_dump(filemtime(          __FILE__) ===
         filemtime('file://'.__FILE__));

var_dump(filectime(          __FILE__) ===
         filectime('file://'.__FILE__));

echo "\n";

var_dump(stat(          __FILE__) ===
         stat('file://'.__FILE__));

var_dump(lstat(          __FILE__) ===
         lstat('file://'.__FILE__));

echo "\n";

echo "file://file should not be treated as a relative path nor a hostname\n";
var_dump(chdir(__DIR__) !== false);
var_dump(is_dir('file://'.basename(__FILE__)));

//echo "file://localhost/dir should work\n";
//var_dump(is_dir('file://localhost' + __DIR__));
