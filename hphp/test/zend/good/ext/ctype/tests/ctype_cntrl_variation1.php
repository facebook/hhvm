<?hh
/* Prototype  : bool ctype_cntrl(mixed $c)
 * Description: Checks for control character(s)
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different data types as $c argument to ctype_cntrl() to test behaviour
 */

// get a class
class classA
{
  public function __toString() {
    return "\n\r\t";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_cntrl() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
\t\r\n
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $c argument
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
       varray[],

       // string data
/*19*/ "\t\r\n",
       '
',
       $heredoc,

       // object data
/*22*/ new classA(),

       // undefined data
/*23*/ @$undefined_var,

       // unset data
/*24*/ @$unset_var,

       // resource variable
/*25*/ $fp
];

// loop through each element of $inputs to check the behavior of ctype_cntrl()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  var_dump( ctype_cntrl($input) );
  $iterator++;
};

fclose($fp);

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
