<?hh
<<__EntryPoint>> function main(): void {
var_dump(gettype(getrusage()));
var_dump(gettype(getrusage(1)));
var_dump(gettype(getrusage(-1)));
echo "Done\n";
}
