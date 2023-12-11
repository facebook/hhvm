<?hh
/* Prototype  : number pow  ( number $base  , number $exp  )
 * Description: Exponential expression.
 * Source code: ext/standard/math.c
 */

<<__EntryPoint>> function main(): void {
echo "*** Testing pow() : usage variations ***\n";


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
       PHP_INT_MAX,

       // float data
/*6*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,

       // null data
/*11*/ NULL,
       null,

       // boolean data
/*13*/ true,
       false,
       TRUE,
       FALSE,

       // empty data
/*17*/ "",
       '',
       vec[],

       // string data
/*20*/ "abcxyz",
       'abcxyz',
       $heredoc,

       // resource variable
/*24*/ $fp
];

// loop through each element of $inputs to check the behaviour of pow()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    var_dump(pow(HH\Lib\Legacy_FIXME\cast_for_exponent($input), 3));
    $iterator++;
};
fclose($fp);
echo "===Done===";
}
