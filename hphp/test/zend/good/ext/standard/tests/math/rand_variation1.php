<?hh
/* Prototype  : int rand  ([ int $min  , int $max ] )
 * Description: Generate a random integer.
 * Source code: ext/standard/rand.c
 */

// get a class
class classA
{
}
<<__EntryPoint>> function main(): void {
echo "*** Testing rand() : usage variations ***\n";


// heredoc string
$heredoc = <<<EOT
abc
xyz
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

$inputs = vec[
       // int data
/*1*/  0,
       1,
       12345,
       -2345,
       2147483647,
];

// loop through each element of $inputs to check the behaviour of rand()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    try { var_dump(rand($input, 100)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    $iterator++;
};
fclose($fp);
echo "===Done===";
}
