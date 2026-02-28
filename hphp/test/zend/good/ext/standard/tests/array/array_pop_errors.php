<?hh
<<__EntryPoint>> function main(): void {
$empty_array = vec[];
$number = 5;
$str = "abc";


/* Various combinations of arrays to be used for the test */
$mixed_array1 = vec[ 1,2,3,4,5,6,7,8,9 ];
$mixed_array = vec[
  vec[],
  null,
  vec[ "One", "_Two", "Three", "Four", "Five" ],
  vec[ 6, "six", 7, "seven", 8, "eight", 9, "nine" ],
  dict[ "a" => "aaa", "A" => "AAA", "c" => "ccc", "d" => "ddd", "e" => "eee" ],
  dict[ "1" => "one", "2" => "two", "3" => "three", "4" => "four", "5" => "five" ],
  dict[ 1 => "one", 2 => "two", 3 => 7, 4 => "four", 5 => "five" ],
  dict[ "f" => "fff", "1" => "one", 4 => 6, "" => "blank", 2 => "float", "F" => "FFF",
         "blank" => "", 3 => 3.7, 5 => 7, 6 => 8.6, '5' => "Five", "4name" => "jonny", "a" => NULL, '' => 3 ],
  vec[ 12, "name", 'age', '45' ],
  vec[ vec["oNe", "tWo", 4], vec[10, 20, 30, 40, 50], vec[] ],
  dict[ "one" => 1, "one" => 2, "three" => 3, 0 => 3, 1 => 4, 3 => 33, 4 => 44, 5 => 5, 6 => 6,
          5 => 54, 5 => 57, "5.4" => 554, "5.7" => 557 ]
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
