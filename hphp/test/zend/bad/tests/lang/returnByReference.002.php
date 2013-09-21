<?php
function &returnRef() {
		global $a;
		return $a;
}

function returnVal() {
		global $a;
		return $a;
}

$a = "original";
$b =& returnVal();
$b = "changed";
var_dump($a); //expecting warning + "original" 

$a = "original";
$b =& returnRef();
$b = "changed";
var_dump($a); //expecting "changed" 
?>