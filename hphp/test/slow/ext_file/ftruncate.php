<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing ftruncate");
fclose($f);

$f = fopen($tempfile, "r+");
ftruncate($f, 7);
fclose($f);

$f = fopen($tempfile, "r");
var_dump(fread($f, 20));

unlink($tempfile);
