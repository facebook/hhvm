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
echo "*** Testing sha1() : unexpected values for 'raw' ***\n";

$string = "Hello World";

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

            // string data
/*17*/      "ABC",
            'abc',
            "0abc",
            "123abc",

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

// loop through each element of $values for 'raw' argument
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1)." --\n";
  // use bin2hex to catch those cases were raw is true
  try { var_dump( bin2hex(sha1($string, $values[$count])) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

//closing resource
fclose($fp);

echo "===DONE===\n";
}
