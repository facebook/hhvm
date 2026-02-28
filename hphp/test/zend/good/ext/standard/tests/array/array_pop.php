<?hh
<<__EntryPoint>> function main(): void {
$empty_array = vec[];
$number = 5;
$str = "abc";


/* Various combinations of arrays to be used for the test */
$mixed_array = vec[
  vec[],
  vec[ 1,2,3,4,5,6,7,8,9 ],
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

/* Loop to test normal functionality with different arrays inputs */
echo "\n*** Normal testing with various array inputs ***\n";

$counter = 1;
foreach( $mixed_array as $sub_array )
{
 echo "\n-- Input Array for Iteration $counter is --\n";
 print_r( $sub_array );
 echo "\nOutput after Pop is :\n";
 var_dump( array_pop(inout $sub_array) );
 $counter++;
}

echo"\nDone";
}
