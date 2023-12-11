<?hh
<<__EntryPoint>> function main(): void {
$array = dict["foo" => "bar", "baz" => 1, "test" => "a ' \" ", 0 => "abc"];
var_dump(http_build_query($array));
var_dump(http_build_query($array, 'foo'));
var_dump(http_build_query($array, 'foo', ';'));
}
