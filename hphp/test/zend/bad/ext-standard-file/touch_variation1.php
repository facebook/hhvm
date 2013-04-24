<?php

$filename = dirname(__FILE__)."/touch.dat";


var_dump(touch($filename, 101));
var_dump(filemtime($filename));
var_dump(fileatime($filename));

@unlink($filename);

@unlink($filename);
var_dump(touch($filename, 100, 102));
var_dump(filemtime($filename));
var_dump(fileatime($filename));

@unlink($filename);
echo "Done\n";

?>