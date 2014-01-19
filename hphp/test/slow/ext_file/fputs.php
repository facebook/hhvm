<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing fputs\n");
fclose($f);

$f = fopen($tempfile, "r");
fpassthru($f);

unlink($tempfile);
