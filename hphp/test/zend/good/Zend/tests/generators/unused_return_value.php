<?hh

function gen($foo) { yield; }
<<__EntryPoint>> function main(): void {
gen('foo'); // return value not used

echo "===DONE===\n";
}
