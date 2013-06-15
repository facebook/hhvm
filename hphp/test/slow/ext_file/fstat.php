<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing fseek");
fclose($f);

$f = fopen($tempfile, "r");
var_dump(fstat($f)['size']);

unlink($tempfile);
