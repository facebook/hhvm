<?hh
/* Prototype  : string stripslashes ( string $str )
 * Description: Returns an un-quoted string
 * Source code: ext/standard/string.c
*/

/*
 * Test stripslashes() with double dimensional arrays
*/

function stripslashes_deep($value)  :mixed{
  $value = is_array($value) ? array_map(stripslashes_deep<>, $value) : stripslashes($value);
  return $value;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing stripslashes() : with double dimensional arrays ***\n";

// initialising the string array

$str_array = vec[
                    vec["", vec[]],
                    vec["", vec[""]],
                    vec["f\\'oo", "b\\'ar", vec["fo\\'o", "b\\'ar"]],
                    vec["f\\'oo", "b\\'ar", vec[""]],
                    vec["f\\'oo", "b\\'ar", vec["fo\\'o", "b\\'ar", vec[""]]],
                    vec["f\\'oo", "b\\'ar", vec["fo\\'o", "b\\'ar", vec["fo\\'o", "b\\'ar"]]]
                  ];
$count = 1;
// looping to test for all strings in $str_array
foreach( $str_array as $arr )  {
  echo "\n-- Iteration $count --\n";
  var_dump( stripslashes_deep($arr) );
  $count ++;
}

echo "Done\n";
}
