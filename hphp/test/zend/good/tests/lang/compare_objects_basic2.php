<?hh

class X {
}


// Set the default time zone.
<<__EntryPoint>> function main(): void {
date_default_timezone_set("Europe/London");
echo "Simple test comparing two objects with different compare callback handler\n";

$obj1 = new X();
$obj2 = new DateTime(("2009-02-12 12:47:41 GMT"));

var_dump($obj1 == $obj2);
echo "===DONE===\n";
}
