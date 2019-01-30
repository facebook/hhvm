<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

echo "Inside file >" . __FILE__ . "< at line >" . __LINE__ .
    "< with namespace >" . __NAMESPACE__ . "<\n";

//var_dump(MY_MIN);
//var_dump(MY_MAX);

// Try to require a non-existant file

$fileName = 'unknown.php';
//$inc = require $fileName;
//echo "require file " . ($inc == 1 ? "does" : "does not") . " exist\n";

// require an existing file that has its own namespace

$fileName = 'limits' . '.php';
$inc = require $fileName;
var_dump($inc);

echo "Inside file >" . __FILE__ . "< at line >" . __LINE__ .
    "< with namespace >" . __NAMESPACE__ . "<\n";

// require another existing file that has its own namespace
    
$inc = require('mycolors.php');
var_dump($inc);

echo "Inside file >" . __FILE__ . "< at line >" . __LINE__ .
    "< with namespace >" . __NAMESPACE__ . "<\n";

echo "----------------------------------\n";

// Try to access constants defined in an included file

if (defined("MY_MIN"))
    echo "MY_MIN is defined with value >" . constant("MY_MIN") . "\n";
else
    echo "MY_MIN is not defined\n";

echo "----------------------------------\n";

// require a file that has no return statement

$inc = require('return_none.php');
var_dump($inc);

// require a file that has a return statement without a return value

$inc = require('return_without_value.php');
var_dump($inc);

// require a file that has a return statement with a return value

$inc = require('return_with_value.php');
var_dump($inc);

echo "----------------------------------\n";

// see how low the precedence of require is

//if (require('return_with_value.php') == 987) ;
if ((require('return_with_value.php')) == 987) ;
//if (require('return_with_value.php') | 987) ;
if ((require('return_with_value.php')) | 987) ;
//if (require('return_with_value.php') && 987) ;
if ((require('return_with_value.php')) && 987) ;
//if (require('return_with_value.php') or 987) ;
if ((require('return_with_value.php')) or 987) ;

echo "----------------------------------\n";

// see if included file can access including file's variables, and if including file
// can access the included file's functions and variables

$v1 = 10;
$v2 = "Hello";
echo "Inside file >" . __FILE__ . "< at line >" . __LINE__ . "<\n";

echo "----------------------------------\n";

$inc = require 'test.php';
var_dump($inc);

echo "----------------------------------\n";

echo "Inside file >" . __FILE__ . "< at line >" . __LINE__ . "<\n";
test();
echo "\$local1: $local1\n";

echo "Inside file >" . __FILE__ . "< at line >" . __LINE__ . "<\n";

echo "----------------------------------\n";

// get the set of included files

print_r(get_included_files());
