<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

file_put_contents($tempfile, 'testing file_put_contents');
var_dump(file_get_contents($tempfile));

unlink($tempfile);

$tempfile2 = tempnam('/tmp', 'vmextfiletest');

file_put_contents('File://' . $tempfile2, 'testing file_put_contents with File:// url and LOCK_EX', LOCK_EX);
var_dump(file_get_contents($tempfile2));

unlink($tempfile2);
