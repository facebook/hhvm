<?hh

<<__EntryPoint>> function main(): void {
echo "Simple testcase for getopt_with_optind() function\n";

$argv = HH\global_get('argv');
$argv[] = '-f';
$argv[] = '-g';
$argv[] = 'value_for_g';
$argv[] = '-k';
$argv[] = 'value_for_k';
$argv[] = '-mno';
HH\global_set('argv', $argv);

$optind = 1;

// Parse `f`
var_dump(getopt_with_optind('f::', null, inout $optind));
var_dump($optind);

// Parse `g`
var_dump(getopt_with_optind('g:', null, inout $optind));
var_dump($optind);

// Parse `k`
var_dump(getopt_with_optind('k:', null, inout $optind));
var_dump($optind);

// Parse `mno`
var_dump(getopt_with_optind('mno::', null, inout $optind));
var_dump($optind);

echo "Done\n";
}