<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

touch($tempfile);
var_dump(file_exists($tempfile));

$tempfile2 = tempnam('/tmp', 'vmextfiletest');

var_dump(file_exists($tempfile2));
copy($tempfile, $tempfile2);
var_dump(file_exists($tempfile2));

$tempfile3 = tempnam('/tmp', 'vmextfiletest');

var_dump(file_exists($tempfile2));
rename($tempfile2, $tempfile3);
var_dump(file_exists($tempfile2));

var_dump(file_exists($tempfile3));

unlink($tempfile);
unlink($tempfile2);
unlink($tempfile3);
