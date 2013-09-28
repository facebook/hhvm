<?php


function &returnVarByRef () {
    $b=1;
	return $b; 
}

function &testReturnVarByRef() {
	return returnVarByRef();
}

function returnVal () {
return 1; 
}

function &testReturnValByRef() {
	return returnVal();
}

echo "\n---> 1. Return a variable by reference -> No warning:\n";

var_dump (testReturnVarByRef());

echo "\n---> 2. Return a value by reference -> Warning:\n";

var_dump (testReturnValByRef());
