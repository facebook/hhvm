<?hh <<__EntryPoint>> function main(): void {
$array = varray[""];
var_dump(isset($array[0]["a"]["b"]));
var_dump(isset($array[0]["a"]["b"]["c"]));
}
