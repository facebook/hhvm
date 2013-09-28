<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
vfprintf($f, "%s %s", array("testing", "vfprintf"));
fclose($f);

$f = fopen($tempfile, "r");
fpassthru($f);
echo "\n";

unlink($tempfile);
