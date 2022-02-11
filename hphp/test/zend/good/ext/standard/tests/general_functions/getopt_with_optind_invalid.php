<?hh

<<__EntryPoint>> function main(): void {
echo "Testcase for getopt_with_optind() function with invalid optind\n";

$argv = HH\global_get('argv');
$argv[] = '-f';
HH\global_set('argv', $argv);

$optind = -1;
var_dump(getopt_with_optind('f::', null, inout $optind));
var_dump($optind);

$optind = -1;
var_dump(getopt_with_optind('g::', null, inout $optind));
var_dump($optind);

$optind = 0;
var_dump(getopt_with_optind('f::', null, inout $optind));
var_dump($optind);

$optind = 10000;
var_dump(getopt_with_optind('f::', null, inout $optind));
var_dump($optind);

$optind = 1000000000000000;
var_dump(getopt_with_optind('f::', null, inout $optind));
var_dump($optind);

echo "Done\n";
}
