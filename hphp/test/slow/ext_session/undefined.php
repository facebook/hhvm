<?php



<<__EntryPoint>>
function main_undefined() {
error_reporting(-1);

// Make sure we don't die trying to read an uninitialized session from C++
session_encode();

var_dump($_SESSION);
var_dump($GLOBALS['_SESSION']);
session_start();
var_dump($_SESSION);
var_dump($GLOBALS['_SESSION']);
}
