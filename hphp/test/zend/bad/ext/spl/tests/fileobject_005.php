<?php

set_include_path(dirname(dirname(__FILE__)));

$path = dirname(__FILE__).DIRECTORY_SEPARATOR.'fileobject_005.txt';
touch($path);

$fo = new SplFileObject('tests'.DIRECTORY_SEPARATOR.'fileobject_005.txt', 'w+', true);
$fo->fwrite("blahlubba");
var_dump($fo->ftruncate(4));

$fo->rewind();
var_dump($fo->fgets(8));

$fo->rewind();
$fo->fwrite("blahlubba");

// This should throw a warning and return NULL since an argument is missing 
var_dump($fo->ftruncate());

?>
==DONE==
<?php
$path = dirname(__FILE__).DIRECTORY_SEPARATOR.'fileobject_005.txt';
unlink($path);
?>