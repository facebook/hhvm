<?hh <<__EntryPoint>> function main(): void {
$read = darray[];
$read[1] = fopen(__FILE__, "r");
$read["myindex"] = reset(inout $read);
$write = NULL;
$except = NULL;

var_dump($read);

stream_select(inout $read, inout $write, inout $except, 0);

var_dump($read);
}
