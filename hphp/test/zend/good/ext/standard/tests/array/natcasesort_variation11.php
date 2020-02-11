<?hh
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays where the keys are different data types to test behaviour of natcasesort()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing natcasesort() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// arrays with keys as different data types to be passed as $array_arg
$inputs = darray[

       // int data
/*1*/  'int' => darray[
       0 => 'zero',
       1 => 'one',
       12345 => 'positive',
       -2345 => 'negative',
       ],

       // float data
/*2*/  'float' => darray[
       10.5 => 'positive',
       -10.5 => 'negative',
       .5 => 'half',
       ],

/*3*/  'extreme floats' => darray[
       12.3456789000e6 => 'large',
       12.3456789000E-10 => 'small',
       ],

       // null data
/*4*/  'null uppercase' => darray[
       NULL => 'null 1',
       ],

/*5*/  'null lowercase' => darray[
       null => 'null 2',
       ],

       // boolean data
/*6*/ 'bool lowercase' => darray[
       true => 'lowert',
       false => 'lowerf',
       ],

/*7*/  'bool uppercase' => darray[
       TRUE => 'uppert',
       FALSE => 'upperf',
       ],

       // empty data
/*8*/ 'empty double quotes' => darray[
       "" => 'emptyd',
       ],

/*9*/  'empty single quotes' => darray[
       '' => 'emptys',
       ],

       // string data
/*10*/ 'string' => darray[
       "stringd" => 'stringd',
       'strings' => 'strings',
       $heredoc => 'stringh',
       ],

       // undefined data
/*11*/ 'undefined' => darray[
       @$undefined_var => 'undefined',
       ],

       // unset data
/*12*/ 'unset' => darray[
       @$unset_var => 'unset',
       ],

       // duplicate values
/*13*/ 'duplicate' => darray[
       'foo' => 'bar',
       'baz' => 'bar',
       'hello' => 'world'
       ],

];

// loop through each element of $inputs to check the behavior of natcasesort()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    var_dump( natcasesort(inout $input) );
    var_dump($input);
    $iterator++;
};

echo "Done";
}
