<?hh <<__EntryPoint>> function main(): void {
var_dump(json_encode('a/b'));
var_dump(json_encode('a/b', JSON_UNESCAPED_SLASHES));
}
