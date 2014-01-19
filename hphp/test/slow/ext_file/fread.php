<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing fread");
fclose($f);

$f = fopen($tempfile, "r");
echo fread($f, 7);
echo fread($f, 100);
echo "\n";

unlink($tempfile);
