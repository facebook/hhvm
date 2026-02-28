<?hh


<<__EntryPoint>>
function main_1450() :mixed{

var_export(dict["\0" => 1]);
var_export(dict["\0" => "\0"]);
var_export(dict["\0" => "\\"]);
var_export(dict["\0" => "\'"]);
var_export(dict["\\" => 1]);
var_export(dict["\\" => "\0"]);
var_export(dict["\\" => "\\"]);
var_export(dict["\\" => "\'"]);
var_export(dict["\'" => 1]);
var_export(dict["\'" => "\0"]);
var_export(dict["\'" => "\\"]);
var_export(dict["\'" => "\'"]);
var_export(dict["\a" => "\a"]);
var_export(!dict["\0" => "\0"]);
var_export((dict["\0" => "\0"]));
var_export((int)dict["\0" => "\0"]);
var_export((int)dict["\0" => "\0"]);
var_export((bool)dict["\0" => "\0"]);
var_export((bool)dict["\0" => "\0"]);
var_export((float)dict["\0" => "\0"]);
var_export((float)dict["\0" => "\0"]);
var_export((float)dict["\0" => "\0"]);
$a = "0x10";
var_export($a);
var_export("\0");
$a = dict["\0" => 1];
var_export($a);
$a = dict["\0" => "\0"];
var_export($a);
$a = dict["\0" => "\\"];
var_export($a);
$a = dict["\0" => "\'"];
var_export($a);
$a = dict["\\" => 1];
var_export($a);
$a = dict["\\" => "\0"];
var_export($a);
$a = dict["\\" => "\\"];
var_export($a);
$a = dict["\\" => "\'"];
var_export($a);
$a = dict["\'" => 1];
var_export($a);
$a = dict["\'" => "\0"];
var_export($a);
$a = dict["\'" => "\\"];
var_export($a);
$a = dict["\'" => "\'"];
var_export($a);
$a = dict["\a" => "\a"];
var_export($a);
}
