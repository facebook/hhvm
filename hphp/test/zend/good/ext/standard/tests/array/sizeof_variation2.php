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
$values = array (
  /* 1  */  array($fp, "resource" => $fp),
            array(1, array(3, 4, array(6, array(8)))),
            array("a" => 1, 'b' => 2, array( "c" =>3, array( "d" => 5))),
            array(),
  /* 5  */  array(1, 2, 3, 4),
            array("Saffron", "White", "Green"),
            array('saffron', 'white', 'green'),
            array(1 => "Hi", 2 => "Hello" ),
            array("color" => "red", "item" => "pen"),
  /* 10 */  array('color' => 'red', 'item' => 'pen'),
            array(TRUE => "red", FALSE => "pen" ),
            array(false => 'red', true => 'pen' ),
            array('color' => "red", "item" => 'pen', 1 => "Hi", "" => "Hello" ),
  /* 14 */  array($fp, "resource1" => $fp, 'resource2' => $fp, array( $fp, 'type' => $fp) )
);   

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
