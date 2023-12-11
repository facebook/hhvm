<?hh
/* Prototype  : number abs  ( mixed $number  )
 * Description: Returns the absolute value of number.
 * Source code: ext/standard/math.c
 */

/*
 * Pass different data types as $number argument to abs() to test behaviour
 */

<<__EntryPoint>> function main(): void {
echo "*** Testing abs() : usage variations ***\n";


// heredoc string
$heredoc = <<<EOT
abs
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $number argument
$inputs = vec[

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
/*19*/ "abs",
       'abs',
       $heredoc,

       // resource variable
/*23*/ $fp
];

// loop through each element of $inputs to check the behavior of abs()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    var_dump(abs($input) );
    $iterator++;
};

fclose($fp);
echo "===Done===";
}
