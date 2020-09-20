<?hh
/* Prototype  : string dechex  ( int $number  )
 * Description: Returns a string containing a hexadecimal representation of the given number  argument.
 * Source code: ext/standard/math.c
 */

// get a class
class classA
{
}
<<__EntryPoint>> function main(): void {
echo "*** Testing dechex() : usage variations ***\n";
//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
abc
xyz
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

$inputs = varray[
       // int data
/*1*/  0,
       1,
       12345,
       -2345,
       PHP_INT_MAX,  // largest decimal
       PHP_INT_MIN,

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
       varray[],

       // string data
/*21*/ "abcxyz",
       'abcxyz',
       $heredoc,

       // object data
/*24*/ new classA(),

       // undefined data
/*25*/ @$undefined_var,

       // unset data
/*26*/ @$unset_var,

       // resource variable
/*27*/ $fp
];

// loop through each element of $inputs to check the behaviour of dechex()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    var_dump(dechex($input));
    $iterator++;
};
fclose($fp);
echo "===Done===";
}
