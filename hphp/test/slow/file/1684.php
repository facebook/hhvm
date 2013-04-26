<?php

$file = __DIR__."/../../sample_dir/symlink";

var_dump(filetype($file));
var_dump(is_link($file));
$a = lstat($file);
var_dump($a['mtime'] > 1234567890);
