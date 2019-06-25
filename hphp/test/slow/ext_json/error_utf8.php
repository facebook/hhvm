<?hh <<__EntryPoint>> function main(): void {
$data = "\xB1\x31";
$data = json_encode($data);
var_dump(json_last_error());
}
