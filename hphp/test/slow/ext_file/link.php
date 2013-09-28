<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');
$tempfile2 = tempnam('/tmp', 'vmextfiletest');

unlink($tempfile2);
var_dump(file_exists($tempfile2));
link($tempfile, $tempfile2);
var_dump(file_exists($tempfile2));

unlink($tempfile2);
var_dump(file_exists($tempfile2));
symlink($tempfile, $tempfile2);
var_dump(file_exists($tempfile2));

unlink($tempfile);
unlink($tempfile2);
