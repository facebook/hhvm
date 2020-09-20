<?hh <<__EntryPoint>> function main(): void {
var_dump(
    preg_match("/foo/i\r", 'FOO'),
    preg_last_error()
);
}
