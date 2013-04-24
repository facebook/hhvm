<?php

set_include_path('tests');

chdir(dirname(dirname(__FILE__))); // ext/spl


$fo = new SplFileObject('fileobject_004.phpt', 'r', true);

var_dump($fo->getPath());
var_dump($fo->getFilename());
var_dump($fo->getRealPath());
?>
==DONE==