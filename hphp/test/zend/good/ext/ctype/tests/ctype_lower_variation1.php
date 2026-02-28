<?hh
/* Prototype  : bool ctype_lower(mixed $c)
 * Description: Checks for lowercase character(s)
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different data types as $c argument to ctype_lower() to test behaviour
 */

// get a class
class classA
{
    public function __toString() :mixed{
        return "class";
    }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_lower() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");


// heredoc string
$heredoc = <<<EOT
heredoc
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $c argument
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
       vec[],

       // string data
/*19*/ "string",
       'string',
       $heredoc,

       // object data
/*22*/ new classA(),



       // resource variable
/*23*/ $fp
];

// loop through each element of $inputs to check the behavior of ctype_lower()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    var_dump( ctype_lower($input) );
    $iterator++;
};

fclose($fp);

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
