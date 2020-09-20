<?hh <<__EntryPoint>> function main(): void {
echo "Basic test of POSIX posix_getrlimit function\n";
var_dump(posix_getrlimit());
echo "===DONE====";
}
