<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fwrite($f, "testing fwrite", 7);
fclose($f);

$f = fopen($tempfile, 'r');
fpassthru($f);
echo "\n";

unlink($tempfile);
