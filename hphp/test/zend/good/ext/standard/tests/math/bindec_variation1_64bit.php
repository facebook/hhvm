<?hh
/* Prototype  : number bindec  ( string $binary_string  )
 * Description: Returns the decimal equivalent of the binary number represented by the binary_string  argument.
 * Source code: ext/standard/math.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing bindec() : usage variations ***\n";

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

       // float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
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
       'Array',

       // string data
/*19*/ "abcxyz",
       'abcxyz',
       $heredoc,



       // resource variable
/*22*/ $fp
];

// loop through each element of $inputs to check the behaviour of bindec()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    var_dump(bindec($input));
    $iterator++;
};
fclose($fp);
echo "===Done===";
}
