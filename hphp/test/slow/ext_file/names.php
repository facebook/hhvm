<?php

var_dump(basename('test/ext/test_ext_file.tmp'));
var_dump(basename('test/ext/test_ext_file.tmp', '.tmp'));

var_dump(fnmatch(__DIR__.'/test_*_file.txt', __DIR__.'/test_ext_file.txt'));

var_dump(glob(__DIR__.'/test_*_file.txt'));
