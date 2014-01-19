<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing\nfile\n");
fclose($f);

$items = file($tempfile);
var_dump($items);

unlink($tempfile);
