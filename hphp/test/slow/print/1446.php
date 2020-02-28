<?hh


// disable array -> "Array" conversion notice
<<__EntryPoint>>
function main_1446() {
error_reporting(error_reporting() & ~E_NOTICE);

print_r(darray["\0" => 1]);
print_r(darray["\0" => "\0"]);
print_r(darray["\0" => "\\"]);
print_r(darray["\0" => "\'"]);
print_r(darray["\\" => 1]);
print_r(darray["\\" => "\0"]);
print_r(darray["\\" => "\\"]);
print_r(darray["\\" => "\'"]);
print_r(darray["\'" => 1]);
print_r(darray["\'" => "\0"]);
print_r(darray["\'" => "\\"]);
print_r(darray["\'" => "\'"]);
print_r(darray["\a" => "\a"]);
print_r(!darray["\0" => "\0"]);
print_r((darray["\0" => "\0"]));
print_r((int)darray["\0" => "\0"]);
print_r((int)darray["\0" => "\0"]);
print_r((bool)darray["\0" => "\0"]);
print_r((bool)darray["\0" => "\0"]);
print_r((float)darray["\0" => "\0"]);
print_r((float)darray["\0" => "\0"]);
print_r((float)darray["\0" => "\0"]);
print_r((string)darray["\0" => "\0"]);
$a = "0x10";
print_r($a);
print_r("\0");
$a = darray["\0" => 1];
print_r($a);
$a = darray["\0" => "\0"];
print_r($a);
$a = darray["\0" => "\\"];
print_r($a);
$a = darray["\0" => "\'"];
print_r($a);
$a = darray["\\" => 1];
print_r($a);
$a = darray["\\" => "\0"];
print_r($a);
$a = darray["\\" => "\\"];
print_r($a);
$a = darray["\\" => "\'"];
print_r($a);
$a = darray["\'" => 1];
print_r($a);
$a = darray["\'" => "\0"];
print_r($a);
$a = darray["\'" => "\\"];
print_r($a);
$a = darray["\'" => "\'"];
print_r($a);
$a = darray["\a" => "\a"];
print_r($a);
}
