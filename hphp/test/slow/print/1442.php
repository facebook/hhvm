<?php


// disable array -> "Array" conversion notice
<<__EntryPoint>>
function main_1442() {
error_reporting(error_reporting() & ~E_NOTICE);

echo(array("\0" => 1));
echo(array("\0" => "\0"));
echo(array("\0" => "\\"));
echo(array("\0" => "\'"));
echo(array("\\" => 1));
echo(array("\\" => "\0"));
echo(array("\\" => "\\"));
echo(array("\\" => "\'"));
echo(array("\'" => 1));
echo(array("\'" => "\0"));
echo(array("\'" => "\\"));
echo(array("\'" => "\'"));
echo(array("\a" => "\a"));
echo(!array("\0" => "\0"));
echo((array("\0" => "\0")));
echo((int)array("\0" => "\0"));
echo((integer)array("\0" => "\0"));
echo((bool)array("\0" => "\0"));
echo((boolean)array("\0" => "\0"));
echo((float)array("\0" => "\0"));
echo((double)array("\0" => "\0"));
echo((real)array("\0" => "\0"));
echo((string)array("\0" => "\0"));
$a = "0x10";
echo($a);
echo("\0");
$a = array("\0" => 1);
echo($a);
$a = array("\0" => "\0");
echo($a);
$a = array("\0" => "\\");
echo($a);
$a = array("\0" => "\'");
echo($a);
$a = array("\\" => 1);
echo($a);
$a = array("\\" => "\0");
echo($a);
$a = array("\\" => "\\");
echo($a);
$a = array("\\" => "\'");
echo($a);
$a = array("\'" => 1);
echo($a);
$a = array("\'" => "\0");
echo($a);
$a = array("\'" => "\\");
echo($a);
$a = array("\'" => "\'");
echo($a);
$a = array("\a" => "\a");
echo($a);
}
