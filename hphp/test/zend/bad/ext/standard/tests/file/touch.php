<?php

// This doesn't work for windows, time, atime usage results in very different
// output to linux. This could be a php.net bug on windows or a windows querk.
$filename = dirname(__FILE__)."/touch.dat";

var_dump(touch());
var_dump(touch($filename));
var_dump(filemtime($filename));
@unlink($filename);
var_dump(touch($filename, 101));
var_dump(filemtime($filename));

@unlink($filename);
var_dump(touch($filename, -1));
var_dump(filemtime($filename));

@unlink($filename);
var_dump(touch($filename, 100, 100));
var_dump(filemtime($filename));

@unlink($filename);
var_dump(touch($filename, 100, -100));
var_dump(filemtime($filename));

var_dump(touch("/no/such/file/or/directory"));

@unlink($filename);

echo "Done\n";
?>