<?hh


<<__EntryPoint>>
function main_1437() :mixed{

var_dump(dict["\0" => 1]);
var_dump(dict["\0" => "\0"]);
var_dump(dict["\0" => "\\"]);
var_dump(dict["\0" => "\'"]);
var_dump(dict["\\" => 1]);
var_dump(dict["\\" => "\0"]);
var_dump(dict["\\" => "\\"]);
var_dump(dict["\\" => "\'"]);
var_dump(dict["\'" => 1]);
var_dump(dict["\'" => "\0"]);
var_dump(dict["\'" => "\\"]);
var_dump(dict["\'" => "\'"]);
var_dump(dict["\a" => "\a"]);
var_dump(!dict["\0" => "\0"]);
var_dump((dict["\0" => "\0"]));
var_dump((int)dict["\0" => "\0"]);
var_dump((int)dict["\0" => "\0"]);
var_dump((bool)dict["\0" => "\0"]);
var_dump((bool)dict["\0" => "\0"]);
var_dump((float)dict["\0" => "\0"]);
var_dump((float)dict["\0" => "\0"]);
var_dump((float)dict["\0" => "\0"]);
$a = "0x10";
var_dump($a);
var_dump("\0");
$a = dict["\0" => 1];
var_dump($a);
$a = dict["\0" => "\0"];
var_dump($a);
$a = dict["\0" => "\\"];
var_dump($a);
$a = dict["\0" => "\'"];
var_dump($a);
$a = dict["\\" => 1];
var_dump($a);
$a = dict["\\" => "\0"];
var_dump($a);
$a = dict["\\" => "\\"];
var_dump($a);
$a = dict["\\" => "\'"];
var_dump($a);
$a = dict["\'" => 1];
var_dump($a);
$a = dict["\'" => "\0"];
var_dump($a);
$a = dict["\'" => "\\"];
var_dump($a);
$a = dict["\'" => "\'"];
var_dump($a);
$a = dict["\a" => "\a"];
var_dump($a);
}
