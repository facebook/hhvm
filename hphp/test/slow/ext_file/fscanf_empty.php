<?php
$tempfile = tempnam('/tmp', 'vmextfiletest');

touch($tempfile);
$f = fopen($tempfile, "r");
$res = fscanf($f, "%s %s");
var_dump($res);

unlink($tempfile);
