<?hh
<<__EntryPoint>> function main(): void {
var_dump(time_nanosleep(-1, 0));
var_dump(time_nanosleep(0, -1));
echo "===DONE===\n";
}
