<?php

$filename = dirname(__FILE__)."/parse_ini_file.dat";
@unlink($filename); /* Make sure the file really does not exist! */

var_dump(parse_ini_file());
var_dump(parse_ini_file(1,1,1,1));
var_dump(parse_ini_file($filename));
var_dump(parse_ini_file($filename, true));

$ini = "
test =
";
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename));
$ini = "
test==
";
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename));

$ini = "
test=test=
";
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename));

$ini = "
test= \"new
line\"
";
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename));

define("TEST_CONST", "test const value");
$ini = "
test=TEST_CONST
";
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename));

$ini = "
[section]
test=hello
";
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename, true));

$ini = "
[section]
test=hello
";
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename, false));

$ini = "
section.test=hello
";
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename, true));

$ini = "
[section]
section.test=hello
";
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename, true));

$ini = "
[section]
1=2
";
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename, true));

$ini = "
1=2
";
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename, true));
$ini = "
test=test2
test=test3
test=test4
";
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename, true));

/* From bug #44574 */
$ini = "[section1]\nname = value";
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename, true));

/* #44842, labels starting with underscore */
$ini = <<<'INI'
foo=bar1
_foo=bar2
foo_=bar3
INI;
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename, true));

/* #44575, comments starting with '#' */
$ini = <<<'INI'
foo=bar1
; comment
_foo=bar2
# comment
foo_=bar3
INI;
file_put_contents($filename, $ini);
var_dump(parse_ini_file($filename, true));


@unlink($filename);
echo "Done\n";
?>