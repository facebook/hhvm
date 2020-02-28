<?hh


// disable array -> "Array" conversion notice
<<__EntryPoint>>
function main_1442() {
error_reporting(error_reporting() & ~E_NOTICE);

echo(darray["\0" => 1]);
echo(darray["\0" => "\0"]);
echo(darray["\0" => "\\"]);
echo(darray["\0" => "\'"]);
echo(darray["\\" => 1]);
echo(darray["\\" => "\0"]);
echo(darray["\\" => "\\"]);
echo(darray["\\" => "\'"]);
echo(darray["\'" => 1]);
echo(darray["\'" => "\0"]);
echo(darray["\'" => "\\"]);
echo(darray["\'" => "\'"]);
echo(darray["\a" => "\a"]);
echo(!darray["\0" => "\0"]);
echo((darray["\0" => "\0"]));
echo((int)darray["\0" => "\0"]);
echo((int)darray["\0" => "\0"]);
echo((bool)darray["\0" => "\0"]);
echo((bool)darray["\0" => "\0"]);
echo((float)darray["\0" => "\0"]);
echo((float)darray["\0" => "\0"]);
echo((float)darray["\0" => "\0"]);
echo((string)darray["\0" => "\0"]);
$a = "0x10";
echo($a);
echo("\0");
$a = darray["\0" => 1];
echo($a);
$a = darray["\0" => "\0"];
echo($a);
$a = darray["\0" => "\\"];
echo($a);
$a = darray["\0" => "\'"];
echo($a);
$a = darray["\\" => 1];
echo($a);
$a = darray["\\" => "\0"];
echo($a);
$a = darray["\\" => "\\"];
echo($a);
$a = darray["\\" => "\'"];
echo($a);
$a = darray["\'" => 1];
echo($a);
$a = darray["\'" => "\0"];
echo($a);
$a = darray["\'" => "\\"];
echo($a);
$a = darray["\'" => "\'"];
echo($a);
$a = darray["\a" => "\a"];
echo($a);
}
