<?hh


// disable array -> "Array" conversion notice
<<__EntryPoint>>
function main_1448() {
error_reporting(error_reporting() & ~E_NOTICE);

serialize(darray["\0" => 1]);
serialize(darray["\0" => "\0"]);
serialize(darray["\0" => "\\"]);
serialize(darray["\0" => "\'"]);
serialize(darray["\\" => 1]);
serialize(darray["\\" => "\0"]);
serialize(darray["\\" => "\\"]);
serialize(darray["\\" => "\'"]);
serialize(darray["\'" => 1]);
serialize(darray["\'" => "\0"]);
serialize(darray["\'" => "\\"]);
serialize(darray["\'" => "\'"]);
serialize(darray["\a" => "\a"]);
serialize(!darray["\0" => "\0"]);
serialize((darray["\0" => "\0"]));
serialize((int)darray["\0" => "\0"]);
serialize((int)darray["\0" => "\0"]);
serialize((bool)darray["\0" => "\0"]);
serialize((bool)darray["\0" => "\0"]);
serialize((float)darray["\0" => "\0"]);
serialize((float)darray["\0" => "\0"]);
serialize((float)darray["\0" => "\0"]);
serialize((string)darray["\0" => "\0"]);
$a = "0x10";
serialize($a);
serialize("\0");
$a = darray["\0" => 1];
serialize($a);
$a = darray["\0" => "\0"];
serialize($a);
$a = darray["\0" => "\\"];
serialize($a);
$a = darray["\0" => "\'"];
serialize($a);
$a = darray["\\" => 1];
serialize($a);
$a = darray["\\" => "\0"];
serialize($a);
$a = darray["\\" => "\\"];
serialize($a);
$a = darray["\\" => "\'"];
serialize($a);
$a = darray["\'" => 1];
serialize($a);
$a = darray["\'" => "\0"];
serialize($a);
$a = darray["\'" => "\\"];
serialize($a);
$a = darray["\'" => "\'"];
serialize($a);
$a = darray["\a" => "\a"];
serialize($a);
}
