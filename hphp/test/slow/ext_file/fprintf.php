<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fprintf($f, "%s %s", "testing", "fprintf");
fclose($f);

$f = fopen($tempfile, "r");
fpassthru($f);
echo "\n";

unlink($tempfile);
