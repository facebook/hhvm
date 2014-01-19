<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing feof");
fclose($f);

$f = fopen($tempfile, "r");
var_dump(feof($f));
echo fread($f, 20);
echo "\n";
var_dump(feof($f));

unlink($tempfile);
