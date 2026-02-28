<?hh <<__EntryPoint>> function main(): void {
$read = dict[];
$read[1] = fopen(__FILE__, "r");
foreach ($read as $value) {}
$read["myindex"] = $value;
$write = NULL;
$except = NULL;

var_dump($read);

stream_select(inout $read, inout $write, inout $except, 0);

var_dump($read);
}
