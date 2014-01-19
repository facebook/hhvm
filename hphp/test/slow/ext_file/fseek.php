<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing fseek");
fclose($f);

$f = fopen($tempfile, "r");
fseek($f, -5, SEEK_END);
echo fread($f, 7);
echo "\n";
fseek($f, 7);
echo fread($f, 7);
echo "\n";

unlink($tempfile);
