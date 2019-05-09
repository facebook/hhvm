<?hh
<<__EntryPoint>> function main(): void {
json_decode("a");
json_encode("");
var_dump(json_last_error_msg());
}
