<?php
$a=array("key1"=>array("key2"=>array()));
$a["key1"]["key2"]["key3"]=&$a;

$b=array("key1"=>array("key2"=>array()));
$b["key1"]["key2"]["key3"]=&$b;

array_merge_recursive($a,$b); 

/* Break recursion */
$a["key1"]["key2"]["key3"] = null;
$b["key1"]["key2"]["key3"] = null;

echo "Done.\n";
?>