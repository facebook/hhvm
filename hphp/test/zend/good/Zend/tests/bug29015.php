<?hh <<__EntryPoint>> function main(): void {
$a = new stdClass();
$x = "";
$a->$x = "string('')";
var_dump($a);
}
