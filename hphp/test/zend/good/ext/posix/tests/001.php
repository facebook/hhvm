<?hh
<<__EntryPoint>> function main() {
var_dump(posix_access(str_repeat('bogus path', 1042)));
}
