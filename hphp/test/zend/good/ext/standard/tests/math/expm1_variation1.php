<?hh

/* Prototype  : float expm1  ( float $arg  )
 * Description: Returns exp(number) - 1, computed in a way that is accurate even
 *              when the value of number is close to zero.
 * Source code: ext/standard/math.c
 */

// get a class
class classA
{
}
<<__EntryPoint>> function main(): void {
echo "*** Testing expm1() : usage variations ***\n";


// heredoc string
$heredoc = <<<EOT
abc
xyz
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $arg argument
$inputs = vec[
       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       12.3456789E4,
       12.3456789E-4,
       .5,

       // null data
/*10*/ NULL,
       null,

       // boolean data
/*12*/ true,
       false,
       TRUE,
       FALSE,

       // empty data
/*16*/ "",
       '',
       vec[],

       // string data
/*19*/ "abcxyz",
       'abcxyz',
       $heredoc,

       // array data
       vec[],
       vec[1,2,4],

       // object data
/*24*/ new classA(),



       // resource variable
/*25*/ $fp
];

// loop through each element of $inputs to check the behaviour of expm1()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    try { var_dump(expm1($input)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    $iterator++;
};
fclose($fp);
echo "===Done===";
}
