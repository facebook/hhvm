<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing fgetc");
fclose($f);

$f = fopen($tempfile, "r");
var_dump(fgetc($f));
var_dump(fgetc($f));
var_dump(fgetc($f));
var_dump(fgetc($f));

unlink($tempfile);
