<?hh
<<__EntryPoint>> function main(): void {
var_dump(array_slice(dict["1" => 1], 0, 1, true));
var_dump(array_slice(Map { "1" => 1}, 0, 1, true));
}
