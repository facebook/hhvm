<?hh <<__EntryPoint>> function main() {
$a = new stdClass();
$x = "";
$a->$x = "string('')";
var_dump($a);
}
