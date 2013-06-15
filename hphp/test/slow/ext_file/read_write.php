<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fwrite($f, "testing read/write", 7);
fclose($f);

$f = fopen($tempfile, "r+");
fseek($f, 8);
fwrite($f, "succeeds");
fseek($f, 8);
var_dump(fread($f, 8));

unlink($tempfile);
