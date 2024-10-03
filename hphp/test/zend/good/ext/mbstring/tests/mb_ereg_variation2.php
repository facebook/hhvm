<?hh
/* Prototype  : int mb_ereg(string $pattern, string $string [, array $registers])
 * Description: Regular expression match for multibyte string
 * Source code: ext/mbstring/php_mbregex.c
 */

/*
 * Test behaviour of mb_ereg() when passed different data types as $string arg
 */

// get a class
class classA
{
    public function __toString() :mixed{
        return "Class A object";
    }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_ereg() : usage variations ***\n";

// Initialise function arguments not being substituted
$pattern = 'str';


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed to $string argument
$inputs = vec[
// empty data
/*16*/ "",
       '',

// string data
/*18*/ "string",
       'string',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of mb_ereg()
$iterator = 1;
$regs = null;
foreach($inputs as $input) {
    if (@is_array($regs)){
        $regs = null;
    }
    echo "\n-- Iteration $iterator --\n";
    try { var_dump( mb_ereg($pattern, $input, inout $regs) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    var_dump($regs);
    $iterator++;
};

echo "Done";
}
