<?hh
<<__EntryPoint>> function main(): void {
    var_dump(posix_ttyname(STDIN));
    var_dump(posix_ttyname(STDERR));
    var_dump(posix_ttyname(STDOUT));
    echo "===DONE===\n";
}
