<?php

// test CGetM
$a = array();
$a[0] = "one";
echo $a["0"] . "\n";

// test SetM
$a = array();
$a["0"] = "two";
echo $a[0] . "\n";

// test IssetM
$a = array("narf");
echo isset($a["0"]) . "\n";

// test UnsetM
$a = array();
$a[0] = "uh oh";
unset($a["0"]);
echo count($a) . "\n";

// make sure normal strings work
$a = array();
$a["not an int"] = "woo";
echo $a["not an int"] . "\n";

// it has to be strictly an integer
$a = array();
$a[0] = "hoo!";
$a["00"] = "this is different";
$a["0 "] = "and this";
echo $a[0] . "\n";

