<?hh
<<__EntryPoint>> function main(): void {
$empty_array = varray[];
$number = 5;
$str = "abc";


/* Various combinations of arrays to be used for the test */
$mixed_array1 = varray[ 1,2,3,4,5,6,7,8,9 ];
$mixed_array = varray[
  varray[],
  null,
  varray[ "One", "_Two", "Three", "Four", "Five" ],
  varray[ 6, "six", 7, "seven", 8, "eight", 9, "nine" ],
  darray[ "a" => "aaa", "A" => "AAA", "c" => "ccc", "d" => "ddd", "e" => "eee" ],
  darray[ "1" => "one", "2" => "two", "3" => "three", "4" => "four", "5" => "five" ],
  darray[ 1 => "one", 2 => "two", 3 => 7, 4 => "four", 5 => "five" ],
  darray[ "f" => "fff", "1" => "one", 4 => 6, "" => "blank", 2.4 => "float", "F" => "FFF",
         "blank" => "", 3.7 => 3.7, 5.4 => 7, 6 => 8.6, '5' => "Five", "4name" => "jonny", "a" => NULL, NULL => 3 ],
  varray[ 12, "name", 'age', '45' ],
  varray[ varray["oNe", "tWo", 4], varray[10, 20, 30, 40, 50], varray[] ],
  darray[ "one" => 1, "one" => 2, "three" => 3, 0 => 3, 1 => 4, 3 => 33, 4 => 44, 5 => 5, 6 => 6,
          5.4 => 54, 5.7 => 57, "5.4" => 554, "5.7" => 557 ]
];

/* Testing Error Conditions */
echo "\n*** Testing Error Conditions ***\n";

/* Zero argument  */
try { var_dump( array_pop() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Scalar argument */
var_dump( array_pop(inout $number) );

/* String argument */
var_dump( array_pop(inout $str) );

/* Invalid Number of arguments */
try { var_dump( array_pop(inout $mixed_array1,$mixed_array[2]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Empty Array as argument */
var_dump( array_pop(inout $empty_array) );

echo"\nDone";
}
