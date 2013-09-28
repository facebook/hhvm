<?php

$tempfile = tempnam('/tmp', 'vmextfiletest');

file_put_contents($tempfile, 'testing file_put_contents');
var_dump(file_get_contents($tempfile));

unlink($tempfile);
