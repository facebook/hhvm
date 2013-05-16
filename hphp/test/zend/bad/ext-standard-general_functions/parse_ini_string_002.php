<?php

var_dump(parse_ini_string());
var_dump(parse_ini_string(1,1,1,1));

$ini = "
test =
";
var_dump(parse_ini_string($ini));
$ini = "
test==
";
var_dump(parse_ini_string($ini));

$ini = "
test=test=
";
var_dump(parse_ini_string($ini));

$ini = "
test= \"new
line\"
";
var_dump(parse_ini_string($ini));

define("TEST_CONST", "test const value");
$ini = "
test=TEST_CONST
";
var_dump(parse_ini_string($ini));

$ini = "
[section]
test=hello
";
var_dump(parse_ini_string($ini, true));

$ini = "
[section]
test=hello
";
var_dump(parse_ini_string($ini, false));

$ini = "
section.test=hello
";
var_dump(parse_ini_string($ini, true));

$ini = "
[section]
section.test=hello
";
var_dump(parse_ini_string($ini, true));

$ini = "
[section]
1=2
";
var_dump(parse_ini_string($ini, true));

$ini = "
1=2
";
var_dump(parse_ini_string($ini, true));
$ini = "
test=test2
test=test3
test=test4
";
var_dump(parse_ini_string($ini, true));

/* From bug #44574 */
$ini = "[section1]\nname = value";
var_dump(parse_ini_string($ini, true));

/* #44842, labels starting with underscore */
$ini = <<<'INI'
foo=bar1
_foo=bar2
foo_=bar3
INI;
var_dump(parse_ini_string($ini, true));

echo "Done\n";
?>