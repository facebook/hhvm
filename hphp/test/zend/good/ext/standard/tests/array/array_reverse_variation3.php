<?hh
/* Prototype  : array array_reverse(array $array [, bool $preserve_keys])
 * Description: Return input as a new array with the order of the entries reversed
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_reverse() by giving
 * different array values for $array argument
*/

//get a class
class classA
{
  public function __toString():mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_reverse() : usage variations ***\n";


//get a resource variable
$fp = fopen(__FILE__, "r");

// get a heredoc string
$heredoc = <<<EOT
Hello world
EOT;

$arrays = varray [
/*1*/  varray[1, 2], // array with default keys and numeric values
       varray[1.1, 2.2], // array with default keys & float values
       varray[ varray[2], varray[1]], // sub arrays
       varray[false,true], // array with default keys and boolean values
       varray[], // empty array
       varray[NULL], // array with NULL
       varray["a","aaaa","b","bbbb","c","ccccc"],

       // associative arrays
/*8*/  darray[1 => "one", 2 => "two", 3 => "three"],  // explicit numeric keys, string values
       darray["one" => 1, "two" => 2, "three" => 3 ],  // string keys & numeric values
       darray[ 1 => 10, 2 => 20, 4 => 40, 3 => 30],  // explicit numeric keys and numeric values
       darray[ "one" => "ten", "two" => "twenty", "three" => "thirty"],  // string key/value
       darray["one" => 1, 2 => "two", 4 => "four"],  //mixed

       // associative array, containing null/empty/boolean values as key/value
/*13*/ darray['' => "NULL", '' => "null", "NULL" => NULL, "null" => null],
       darray[1 => "true", 0 => "false", "false" => false, "true" => true],
       darray["" => "emptyd", '' => 'emptys', "emptyd" => "", 'emptys' => ''],
       darray[1 => '', 2 => "", 3 => NULL, 4 => null, 5 => false, 6 => true],
       darray['' => 1, "" => 2, '' => 3, '' => 4, 0 => 5, 1 => 6],

       // array with repetative keys
/*18*/ darray["One" => 1, "two" => 2, "One" => 10, "two" => 20, "three" => 3]
];

// loop through the various elements of $arrays to test array_reverse()
$iterator = 1;
foreach($arrays as $array) {
  echo "-- Iteration $iterator --\n";
  // with default argument
  echo "- with default argument -\n";
  var_dump( array_reverse($array) );
  // with all possible arguments
  echo "- with \$preserve keys = true -\n";
  var_dump( array_reverse($array, true) );
  echo "- with \$preserve_keys = false -\n";
  var_dump( array_reverse($array, false) );
  $iterator++;
};

// close the file resource used
fclose($fp);

echo "Done";
}
