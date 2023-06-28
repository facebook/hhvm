<?hh


<<__EntryPoint>>
function main_1450() :mixed{

var_export(darray["\0" => 1]);
var_export(darray["\0" => "\0"]);
var_export(darray["\0" => "\\"]);
var_export(darray["\0" => "\'"]);
var_export(darray["\\" => 1]);
var_export(darray["\\" => "\0"]);
var_export(darray["\\" => "\\"]);
var_export(darray["\\" => "\'"]);
var_export(darray["\'" => 1]);
var_export(darray["\'" => "\0"]);
var_export(darray["\'" => "\\"]);
var_export(darray["\'" => "\'"]);
var_export(darray["\a" => "\a"]);
var_export(!darray["\0" => "\0"]);
var_export((darray["\0" => "\0"]));
var_export((int)darray["\0" => "\0"]);
var_export((int)darray["\0" => "\0"]);
var_export((bool)darray["\0" => "\0"]);
var_export((bool)darray["\0" => "\0"]);
var_export((float)darray["\0" => "\0"]);
var_export((float)darray["\0" => "\0"]);
var_export((float)darray["\0" => "\0"]);
$a = "0x10";
var_export($a);
var_export("\0");
$a = darray["\0" => 1];
var_export($a);
$a = darray["\0" => "\0"];
var_export($a);
$a = darray["\0" => "\\"];
var_export($a);
$a = darray["\0" => "\'"];
var_export($a);
$a = darray["\\" => 1];
var_export($a);
$a = darray["\\" => "\0"];
var_export($a);
$a = darray["\\" => "\\"];
var_export($a);
$a = darray["\\" => "\'"];
var_export($a);
$a = darray["\'" => 1];
var_export($a);
$a = darray["\'" => "\0"];
var_export($a);
$a = darray["\'" => "\\"];
var_export($a);
$a = darray["\'" => "\'"];
var_export($a);
$a = darray["\a" => "\a"];
var_export($a);
}
