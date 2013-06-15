<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing fscanf");
fclose($f);

$f = fopen($tempfile, "r");
$res = fscanf($f, "%s %s");
var_dump($res);

unlink($tempfile);
