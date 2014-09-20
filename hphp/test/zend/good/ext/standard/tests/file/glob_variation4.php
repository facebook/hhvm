<?php
$path = dirname(__FILE__);

ini_set('open_basedir', $path);

var_dump(glob("$path/*.none"));
var_dump(glob("$path/?.none"));
var_dump(glob("$path/*{hello,world}.none"));
var_dump(glob("$path/*/nothere"));
var_dump(glob("$path/[aoeu]*.none"));
var_dump(glob("$path/directly_not_exists"));

var_dump($path == ini_get('open_basedir'));
?>
==DONE==
