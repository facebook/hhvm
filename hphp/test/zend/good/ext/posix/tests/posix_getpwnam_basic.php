<?hh <<__EntryPoint>> function main(): void {
echo "Basic test of POSIX posix_getpwnam function\n";

var_dump(posix_getpwnam('1'));
var_dump(posix_getpwnam(''));
echo "===DONE====";
}
