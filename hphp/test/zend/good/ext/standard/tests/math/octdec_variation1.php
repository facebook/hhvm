<?hh
/* Prototype  : number octdec  ( string $octal_string  )
 * Description: Returns the decimal equivalent of the octal number represented by the octal_string  argument.
 * Source code: ext/standard/math.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing octdec() : usage variations ***\n";

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
       4294967295,  // largest decimal
       4294967296,

       // float data
/*7*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,

       // null data
/*12*/ NULL,
       null,

       // boolean data
/*14*/ true,
       false,
       TRUE,
       FALSE,

       // empty data
/*18*/ "",
       '',
       'Array',

       // string data
/*21*/ "abcxyz",
       'abcxyz',
       $heredoc,



       // resource variable
/*22*/ $fp
];

// loop through each element of $inputs to check the behaviour of octdec()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    var_dump(octdec($input));
    $iterator++;
};
fclose($fp);
echo "---Done---";
}
