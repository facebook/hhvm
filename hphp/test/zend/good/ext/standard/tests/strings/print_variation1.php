<?hh

/* Prototype  : int print  ( string $arg  )
 * Description: Output a string
 * Source code: n/a, print is a language construct not an extension function
 * Test based on php.net manual example.
*/

<<__EntryPoint>> function main(): void {
echo "*** Testing print() function: with unexpected inputs for 'arg' argument ***\n";


//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values for $input
$inputs =  vec[

          // integer values
/*1*/      0,
          1,
          -2,
          2147483647,
          -2147483648,

          // float values
/*6*/      10.5,
          -20.5,
          10.1234567e10,

          // array values
/*9*/     'Array',
          'Array',
          'Array',

          // boolean values
/*12*/      true,
          false,
          TRUE,
          FALSE,

          // null vlaues
/*16*/      NULL,
          null,

          // resource
/*18*/      $file_handle,
];

// loop through with each element of the $inputs array to test print() function
$count = 1;
foreach($inputs as $input) {
  echo "-- Iteration $count --\n";
  $res = print($input);
  echo "\n";
  var_dump($res);
  $count ++;
}

fclose($file_handle);  //closing the file handle

echo "===DONE===\n";
}
