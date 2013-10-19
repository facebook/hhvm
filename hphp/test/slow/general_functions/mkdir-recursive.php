<?php

$dirName = "mkdirRecursive";
@rmdir($dirName);
mkdir($dirName, 777);
var_dump(@mkdir($dirName, 777, true));
rmdir($dirName);
