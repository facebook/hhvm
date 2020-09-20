<?hh

/* Prototype: string sha1  ( string $str  [, bool $raw_output  ] )
 * Description: Calculate the sha1 hash of a string
 */
//defining class for object variable
class MyClass
{
  public function __toString()
  {
    return "object";
  }
}

<<__EntryPoint>> function main(): void {
echo "*** Testing sha1() : unexpected values for 'str' ***\n";

$raw = false;

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//resource variable
$fp = fopen(__FILE__, 'r');

//different values for 'str' argument
$values = varray[

          // int data
/*1*/      0,
          1,
          12345,
          -2345,

          // float data
/*5*/      10.5,
          -10.5,
          10.1234567e10,
          10.1234567E-10,
          .5,

          // array data
/*10*/      varray[],
          varray[0],
          varray[1],
          varray[1, 2],
          darray['color' => 'red', 'item' => 'pen'],

          // null data
/*15*/      NULL,
          null,

          // boolean data
/*17*/      true,
          false,
          TRUE,
          FALSE,

          // empty data
/*21*/      "",
          '',

          // object data
/*23*/      new MyClass(),

          // undefined data
/*24*/      @$undefined_var,

          // unset data
/*25*/      @$unset_var,

          //resource data
/*26*/      $fp
];

// loop through each element of $values for 'str' argument
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1)." --\n";
  try { var_dump( sha1($values[$count], $raw) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

//closing resource
fclose($fp);

echo "===DONE===\n";
}
