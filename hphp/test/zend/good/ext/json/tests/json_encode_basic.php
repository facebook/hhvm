<?hh
/* Prototype  : string json_encode  ( mixed $value  )
 * Description: Returns the JSON representation of a value
 * Source code: ext/json/php_json.c
 * Alias to functions:
 */
class sample  {
}
<<__EntryPoint>> function main(): void {
echo "*** Testing json_encode() : basic functionality ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a resource variable
$fp = fopen(__FILE__, "r");

// get an object
$obj = new sample();
$obj->MyInt = 99;
$obj->MyFloat = 123.45;
$obj->MyBool = true;
$obj->MyNull = null;
$obj->MyString = "Hello World";

// array with different values for $string
$inputs =  varray [

        // integers
/*1*/    0,
          123,
          -123,
          2147483647,
          -2147483648,

          // floats
/*6*/    123.456,
          1.23E3,
          -1.23E3,

          // boolean
/*9*/    TRUE,
          true,
          FALSE,
          false,

        // NULL
/*13*/    NULL,
          null,

          // strings
/*15*/    "abc",
        'abc',
          "Hello\t\tWorld\n",

          // arrays
/*18*/    varray[],
          varray[1,2,3,4,5],
         darray[1 => "Sun", 2=>"Mon", 3 => "Tue", 4 => "Wed", 5 => "Thur", 6 => "Fri", 7 => "Sat"],
          darray["Jan" => 31, "Feb" => 29, "Mar" => 31, "April" => 30, "May" => 31, "June" => 30],

       // empty data
/*22*/ "",
       '',

       // undefined data
/*24*/ @$undefined_var,

       // unset data
/*25*/ @$unset_var,

       // resource variable
/*26*/ $fp,

      // object variable
/*27*/ $obj

];

// loop through with each element of the $inputs array to test json_encode() function
$count = 1;
foreach($inputs as $input) {
  echo "-- Iteration $count --\n";
  var_dump(json_encode($input));
  $count ++;
}
echo "===Done===";
}
