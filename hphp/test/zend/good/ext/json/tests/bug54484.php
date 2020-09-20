<?hh <<__EntryPoint>> function main(): void {
json_decode('{"test":"test"}');
var_dump(json_last_error());

json_decode("");
var_dump(json_last_error());


json_decode("invalid json");
var_dump(json_last_error());


json_decode("");
var_dump(json_last_error());
}
