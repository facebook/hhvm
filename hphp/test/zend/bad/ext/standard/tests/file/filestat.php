<?php

var_dump(fileinode("."));
var_dump(fileowner("."));
var_dump(filegroup("."));
var_dump(fileatime("."));
var_dump(filectime("."));

var_dump(fileinode("./.."));
var_dump(fileowner("./.."));
var_dump(filegroup("./.."));
var_dump(fileatime("./.."));
var_dump(filectime("./.."));

var_dump(fileinode(__FILE__));
var_dump(fileowner(__FILE__));
var_dump(filegroup(__FILE__));
var_dump(fileatime(__FILE__));
var_dump(filectime(__FILE__));

var_dump(fileinode("/no/such/file/or/dir"));
var_dump(fileowner("/no/such/file/or/dir"));
var_dump(filegroup("/no/such/file/or/dir"));
var_dump(fileatime("/no/such/file/or/dir"));
var_dump(filectime("/no/such/file/or/dir"));

echo "Done\n";
?>