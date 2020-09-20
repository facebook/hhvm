<?hh
/* Prototype  : int mb_ereg(string $pattern, string $string [, array $registers])
 * Description: Regular expression match for multibyte string
 * Source code: ext/mbstring/php_mbregex.c
 */

/*
 * Pass different data types to $pattern argument
 */

// get a class
class classA
{
    public function __toString() {
        return "Class A object";
    }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_ereg() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)
$string = 'string value';

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $pattern argument
$inputs = varray[

// int data
/*1*/  0,
       1,
       12345,
       -2345,

// float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,

// boolean data
/*10*/ true,
       TRUE,

// string data
/*12*/ "string",
       'string',
       $heredoc,

// object data
/*15*/ new classA(),

// resource variable
/*16*/ $fp
];

// loop through each element of $inputs to check the behavior of mb_ereg()
$iterator = 1;
$regs = null;
foreach($inputs as $input) {
    if (@is_array($regs)){
        $regs = null;
    }
    echo "\n-- Iteration $iterator --\n";
    var_dump( mb_ereg($input, $string, inout $regs) );
    var_dump($regs);
    $iterator++;
};

fclose($fp);

echo "Done";
}
