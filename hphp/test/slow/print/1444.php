<?php


// disable array -> "Array" conversion notice
<<__EntryPoint>>
function main_1444() {
error_reporting(error_reporting() & ~E_NOTICE);

print(array("\0" => 1));
print(array("\0" => "\0"));
print(array("\0" => "\\"));
print(array("\0" => "\'"));
print(array("\\" => 1));
print(array("\\" => "\0"));
print(array("\\" => "\\"));
print(array("\\" => "\'"));
print(array("\'" => 1));
print(array("\'" => "\0"));
print(array("\'" => "\\"));
print(array("\'" => "\'"));
print(array("\a" => "\a"));
print(!array("\0" => "\0"));
print((array("\0" => "\0")));
print((int)array("\0" => "\0"));
print((integer)array("\0" => "\0"));
print((bool)array("\0" => "\0"));
print((boolean)array("\0" => "\0"));
print((float)array("\0" => "\0"));
print((double)array("\0" => "\0"));
print((real)array("\0" => "\0"));
print((string)array("\0" => "\0"));
$a = "0x10";
print($a);
print("\0");
$a = array("\0" => 1);
print($a);
$a = array("\0" => "\0");
print($a);
$a = array("\0" => "\\");
print($a);
$a = array("\0" => "\'");
print($a);
$a = array("\\" => 1);
print($a);
$a = array("\\" => "\0");
print($a);
$a = array("\\" => "\\");
print($a);
$a = array("\\" => "\'");
print($a);
$a = array("\'" => 1);
print($a);
$a = array("\'" => "\0");
print($a);
$a = array("\'" => "\\");
print($a);
$a = array("\'" => "\'");
print($a);
$a = array("\a" => "\a");
print($a);
}
