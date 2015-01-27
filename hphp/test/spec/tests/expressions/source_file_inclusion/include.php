<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

echo "Inside file >" . __FILE__ . "< at line >" . __LINE__ .
    "< with namespace >" . __NAMESPACE__ . "<\n";

//var_dump(MY_MIN);
//var_dump(MY_MAX);

// Try to include a non-existant file

$fileName = 'unknown.php';
$inc = include $fileName;
echo "include file " . ($inc == 1 ? "does" : "does not") . " exist\n";

// Include an existing file that has its own namespace

$fileName = 'limits' . '.php';
$inc = include $fileName;
var_dump($inc);

echo "Inside file >" . __FILE__ . "< at line >" . __LINE__ .
    "< with namespace >" . __NAMESPACE__ . "<\n";

// Include another existing file that has its own namespace
    
$inc = include('mycolors.php');
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

// Include a file that has no return statement

$inc = include('return_none.php');
var_dump($inc);

// Include a file that has a return statement without a return value

$inc = include('return_without_value.php');
var_dump($inc);

// Include a file that has a return statement with a return value

$inc = include('return_with_value.php');
var_dump($inc);

echo "----------------------------------\n";

// see how low the precedence of include is

//if (include('return_with_value.php') == 987) ;
if ((include('return_with_value.php')) == 987) ;
//if (include('return_with_value.php') | 987) ;
if ((include('return_with_value.php')) | 987) ;
//if (include('return_with_value.php') && 987) ;
if ((include('return_with_value.php')) && 987) ;
//if (include('return_with_value.php') or 987) ;
if ((include('return_with_value.php')) or 987) ;

echo "----------------------------------\n";

// see if included file can access including file's variables, and if including file
// can access the included file's functions and variables

$v1 = 10;
$v2 = "Hello";
echo "Inside file >" . __FILE__ . "< at line >" . __LINE__ . "<\n";

echo "----------------------------------\n";

$inc = include 'test.php';
var_dump($inc);

echo "----------------------------------\n";

echo "Inside file >" . __FILE__ . "< at line >" . __LINE__ . "<\n";
test();
echo "\$local1: $local1\n";

echo "Inside file >" . __FILE__ . "< at line >" . __LINE__ . "<\n";

echo "----------------------------------\n";

// get the set of included files

print_r(get_included_files());
