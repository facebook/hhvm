<?hh


// disable array -> "Array" conversion notice
<<__EntryPoint>>
function main_1444() {
error_reporting(error_reporting() & ~E_NOTICE);

print(darray["\0" => 1]);
print(darray["\0" => "\0"]);
print(darray["\0" => "\\"]);
print(darray["\0" => "\'"]);
print(darray["\\" => 1]);
print(darray["\\" => "\0"]);
print(darray["\\" => "\\"]);
print(darray["\\" => "\'"]);
print(darray["\'" => 1]);
print(darray["\'" => "\0"]);
print(darray["\'" => "\\"]);
print(darray["\'" => "\'"]);
print(darray["\a" => "\a"]);
print(!darray["\0" => "\0"]);
print((darray["\0" => "\0"]));
print((int)darray["\0" => "\0"]);
print((int)darray["\0" => "\0"]);
print((bool)darray["\0" => "\0"]);
print((bool)darray["\0" => "\0"]);
print((float)darray["\0" => "\0"]);
print((float)darray["\0" => "\0"]);
print((float)darray["\0" => "\0"]);
print((string)darray["\0" => "\0"]);
$a = "0x10";
print($a);
print("\0");
$a = darray["\0" => 1];
print($a);
$a = darray["\0" => "\0"];
print($a);
$a = darray["\0" => "\\"];
print($a);
$a = darray["\0" => "\'"];
print($a);
$a = darray["\\" => 1];
print($a);
$a = darray["\\" => "\0"];
print($a);
$a = darray["\\" => "\\"];
print($a);
$a = darray["\\" => "\'"];
print($a);
$a = darray["\'" => 1];
print($a);
$a = darray["\'" => "\0"];
print($a);
$a = darray["\'" => "\\"];
print($a);
$a = darray["\'" => "\'"];
print($a);
$a = darray["\a" => "\a"];
print($a);
}
