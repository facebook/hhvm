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
  /* 1  */  darray[0 => $fp, "resource" => $fp],
            varray[1, varray[3, 4, varray[6, varray[8]]]],
            darray["a" => 1, 'b' => 2, 0 => darray[ "c" =>3, 0 => darray[ "d" => 5]]],
            varray[],
  /* 5  */  varray[1, 2, 3, 4],
            varray["Saffron", "White", "Green"],
            varray['saffron', 'white', 'green'],
            darray[1 => "Hi", 2 => "Hello" ],
            darray["color" => "red", "item" => "pen"],
  /* 10 */  darray['color' => 'red', 'item' => 'pen'],
            darray[TRUE => "red", FALSE => "pen" ],
            darray[false => 'red', true => 'pen' ],
            darray['color' => "red", "item" => 'pen', 1 => "Hi", "" => "Hello" ],
  /* 14 */  darray[0 => $fp, "resource1" => $fp, 'resource2' => $fp, 1 => darray[0 => $fp, 'type' => $fp] ]
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
