<?hh

function test($a = 0) {
 $b = $a;
 $c = $b[$a];
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
