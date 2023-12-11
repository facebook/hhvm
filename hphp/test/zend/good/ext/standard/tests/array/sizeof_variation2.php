<?hh
/* Prototype  : int sizeof($mixed var)
 * Description: Counts an elements in an array. If Standard PHP library is installed,
 * it will return the properties of an object.
 *
 * Alias to functions: count()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing sizeof() : usage variations ***\n";

// get a resource variable
$fp = fopen(__FILE__, "r");

echo "--- Testing sizeof() with different array values for 'var' argument ---\n";

// array containing different types of array values for 'var' argument
$values = varray [
  /* 1  */  dict[0 => $fp, "resource" => $fp],
            vec[1, vec[3, 4, vec[6, vec[8]]]],
            dict["a" => 1, 'b' => 2, 0 => dict[ "c" =>3, 0 => dict[ "d" => 5]]],
            vec[],
  /* 5  */  vec[1, 2, 3, 4],
            vec["Saffron", "White", "Green"],
            vec['saffron', 'white', 'green'],
            dict[1 => "Hi", 2 => "Hello" ],
            dict["color" => "red", "item" => "pen"],
  /* 10 */  dict['color' => 'red', 'item' => 'pen'],
            dict[1 => "red", 0 => "pen" ],
            dict[0 => 'red', 1 => 'pen' ],
            dict['color' => "red", "item" => 'pen', 1 => "Hi", "" => "Hello" ],
  /* 14 */  dict[0 => $fp, "resource1" => $fp, 'resource2' => $fp, 1 => dict[0 => $fp, 'type' => $fp] ]
];

// loop through each element of the values array for 'var' argument
// check the working of sizeof()
$counter = 1;
for($i = 0; $i < count($values); $i++)
{
  echo "-- Iteration $counter --\n";
  $var = $values[$i];

  echo "Default Mode: ";
  var_dump( sizeof($var) );
  echo "\n";

  $counter++;
}

echo "Done";
}
