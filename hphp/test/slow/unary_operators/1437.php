<?hh


<<__EntryPoint>>
function main_1437() :mixed{

var_dump(darray["\0" => 1]);
var_dump(darray["\0" => "\0"]);
var_dump(darray["\0" => "\\"]);
var_dump(darray["\0" => "\'"]);
var_dump(darray["\\" => 1]);
var_dump(darray["\\" => "\0"]);
var_dump(darray["\\" => "\\"]);
var_dump(darray["\\" => "\'"]);
var_dump(darray["\'" => 1]);
var_dump(darray["\'" => "\0"]);
var_dump(darray["\'" => "\\"]);
var_dump(darray["\'" => "\'"]);
var_dump(darray["\a" => "\a"]);
var_dump(!darray["\0" => "\0"]);
var_dump((darray["\0" => "\0"]));
var_dump((int)darray["\0" => "\0"]);
var_dump((int)darray["\0" => "\0"]);
var_dump((bool)darray["\0" => "\0"]);
var_dump((bool)darray["\0" => "\0"]);
var_dump((float)darray["\0" => "\0"]);
var_dump((float)darray["\0" => "\0"]);
var_dump((float)darray["\0" => "\0"]);
$a = "0x10";
var_dump($a);
var_dump("\0");
$a = darray["\0" => 1];
var_dump($a);
$a = darray["\0" => "\0"];
var_dump($a);
$a = darray["\0" => "\\"];
var_dump($a);
$a = darray["\0" => "\'"];
var_dump($a);
$a = darray["\\" => 1];
var_dump($a);
$a = darray["\\" => "\0"];
var_dump($a);
$a = darray["\\" => "\\"];
var_dump($a);
$a = darray["\\" => "\'"];
var_dump($a);
$a = darray["\'" => 1];
var_dump($a);
$a = darray["\'" => "\0"];
var_dump($a);
$a = darray["\'" => "\\"];
var_dump($a);
$a = darray["\'" => "\'"];
var_dump($a);
$a = darray["\a" => "\a"];
var_dump($a);
}
