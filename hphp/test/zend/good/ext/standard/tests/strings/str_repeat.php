<?hh
/* Prototype: string str_repeat ( string $input, int $multiplier );
   Description: Returns input repeated multiplier times. multiplier has to be
     greater than or equal to 0. If the multiplier is set to 0, the function
     will return an empty string.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing str_repeat() with possible strings ***";
$variations = vec[
  'a',
  'foo',
  'barbazbax',
  "\x00",
  '\0',
  NULL,
  TRUE,
  4,
  1.23,
  "",
  " "
];

/* variations in string and multiplier as an integer */
foreach($variations as $input) {
  $input__str = (string)($input);
  echo "\n--- str_repeat() of '$input__str' ---\n" ;
  for($n=0; $n<4; $n++) {
    echo "-- after repeating $n times is => ";
    try  { echo str_repeat($input, $n)."\n"; } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  }
}

/* variations in multiplier as well as string to be repeated. Same varient
   values are used as string to be repeated as well as multiplier */
echo "\n\n*** Testing str_repeat() with various strings & multiplier value ***";
foreach ( $variations as $input ) {
  $input__str = (string)($input);
  echo "\n--- str_repeat() of '$input__str' ---\n" ;
  foreach ( $variations as $multiplier ) {
    $multiplier__str = (string)($multiplier);
    echo "-- after repeating '$multiplier__str' times is => ";
    try { var_dump( str_repeat($input, $multiplier) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  }
}


echo "\n*** Testing str_repeat() with complex strings containing
       other than 7-bit chars ***\n";
$str = chr(0).chr(128).chr(129).chr(234).chr(235).chr(254).chr(255);
var_dump(str_repeat($str, 3));


echo "\n\n*** Testing error conditions ***";
try { var_dump( str_repeat() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // Zero args
try { var_dump( str_repeat($input[0]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args < expected
try { var_dump( str_repeat($input[0], 3, 4) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected
try { var_dump( str_repeat($input[0], -1) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // $repeat_count < 0

echo "Done\n";
}
