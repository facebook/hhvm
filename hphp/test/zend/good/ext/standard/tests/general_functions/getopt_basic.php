<?hh

<<__EntryPoint>> function main(): void {
echo "Simple testcase for getopt() function\n";

$argv = HH\global_get('argv');
$argv[] = '-f';
$argv[] = '-g';
$argv[] = 'value_for_g';
$argv[] = '-k';
$argv[] = 'value_for_k';
$argv[] = '-mno';
HH\global_set('argv', $argv);

// Parse `f`
var_dump(getopt('f::'));

// Try to parse `g`
var_dump(getopt('g:'));

// Try to parse `k`
var_dump(getopt('k:'));

// Try to parse `mno`
var_dump(getopt('mno::'));

echo "Done\n";
}
