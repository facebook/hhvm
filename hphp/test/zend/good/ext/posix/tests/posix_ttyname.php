<?hh
<<__EntryPoint>> function main(): void {
    var_dump(posix_ttyname(HH\stdin()));
    var_dump(posix_ttyname(HH\stderr()));
    var_dump(posix_ttyname(HH\stdout()));
    echo "===DONE===\n";
}
