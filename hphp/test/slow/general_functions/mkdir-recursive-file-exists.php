<?php

$dirName = "mkdirRecursiveFileExists";
@unlink($dirName);
fopen($dirName, "w");
var_dump(@mkdir($dirName, 777, true));
unlink($dirName);
