<?hh

function gen($foo) :AsyncGenerator<mixed,mixed,void>{ yield; }
<<__EntryPoint>> function main(): void {
gen('foo'); // return value not used

echo "===DONE===\n";
}
