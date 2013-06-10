<?php

echo "-- (1)\n";
var_dump(filter_var_array(NULL));
var_dump(filter_var_array(array()));
var_dump(filter_var_array(array(1,"blah"=>"hoho")));
var_dump(filter_var_array(array(), -1));
var_dump(filter_var_array(array(), 1000000));
var_dump(filter_var_array(array(), ""));

echo "-- (2)\n";
var_dump(filter_var_array(array(""=>""), -1));
var_dump(filter_var_array(array(""=>""), 1000000));
var_dump(filter_var_array(array(""=>""), ""));

echo "-- (3)\n";
var_dump(filter_var_array(array("aaa"=>"bbb"), -1));
var_dump(filter_var_array(array("aaa"=>"bbb"), 1000000));
var_dump(filter_var_array(array("aaa"=>"bbb"), ""));

echo "-- (4)\n";
var_dump(filter_var_array(array(), new stdclass));
var_dump(filter_var_array(array(), array()));
var_dump(filter_var_array(array(), array("var_name"=>1)));
var_dump(filter_var_array(array(), array("var_name"=>-1)));
var_dump(filter_var_array(array("var_name"=>""), array("var_name"=>-1)));

echo "-- (5)\n";
var_dump(filter_var_array(array("var_name"=>""), array("var_name"=>-1, "asdas"=>"asdasd", "qwe"=>"rty", ""=>"")));
var_dump(filter_var_array(array("asdas"=>"text"), array("var_name"=>-1, "asdas"=>"asdasd", "qwe"=>"rty", ""=>"")));


$a = array(""=>""); $b = -1;
var_dump(filter_var_array($a, $b));
var_dump($a, $b);

$a = array(""=>""); $b = 100000;
var_dump(filter_var_array($a, $b));
var_dump($a, $b);

$a = array(""=>""); $b = "";
var_dump(filter_var_array($a, $b));
var_dump($a, $b);

echo "Done\n";
?>