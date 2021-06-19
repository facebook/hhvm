<?hh

class Bar {
}
<<__EntryPoint>> function main(): void {
$b = new Bar;

var_dump($b == NULL);
var_dump($b != NULL);
var_dump($b == true);
var_dump($b != true);
var_dump($b == false);
var_dump($b != false);
var_dump($b == "");
var_dump($b != "");


echo "Done\n";
}
