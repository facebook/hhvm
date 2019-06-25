<?hh
<<__EntryPoint>> function main(): void {
var_dump(posix_access(str_repeat('bogus path', 1042)));
}
