<?hh <<__EntryPoint>> function main(): void {
$correct = 'abcdef';
$wrong = $correct;
$x = vec[];
$x[] = 'x';
$t = 'x';

var_dump($correct);
var_dump($wrong);

$correct[1] = '*';
$correct[3] = '*';
$correct[5] = '*';

// This produces the
$wrong[5] = '*';
$wrong[3] = $wrong[5];
$wrong[1] = $wrong[3];

var_dump($correct);
var_dump($wrong);
}
