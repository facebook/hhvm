<?hh

/* Prototype  : string escapeshellarg  ( string $arg  )
 * Description:  Escape a string to be used as a shell argument.
 * Source code: ext/standard/exec.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing escapeshellarg() : usage variations ***\n";

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
       12,
       -12,
       2147483647,

       // float data
/*6*/  10.5,
       -10.5,
       1.234567e2,
       1.234567E-2,
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

       // undefined data
/*19*/ @$undefined_var,

       // unset data
/*20*/ @$unset_var,

];

// loop through each element of $inputs to check the behaviour of escapeshellarg()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    try { var_dump(escapeshellarg($input)); } catch (Exception $e) { var_dump($e->getMessage()); }
    $iterator++;
};
echo "===Done===";
}
