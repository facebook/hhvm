<?hh
/* Prototype: array array_change_key_case ( array $input [, int $case] )
   Description: Changes the keys in the input array to be all lowercase
   or uppercase. The change depends on the last optional case parameter.
   You can pass two constants there, CASE_UPPER and CASE_LOWER(default).
   The function will leave number indices as is.
*/
<<__EntryPoint>> function main(): void {
$arrays = vec[
  vec[],
  vec[0],
  vec[1],
  vec[-1],
  vec[0, 2, 3, 4, 5],
  vec[1, 2, 3, 4, 5],
  dict["" => 1],
  dict["a" => 1],
  dict["Z" => 1],
  dict["one" => 1],
  dict["ONE" => 1],
  dict["OnE" => 1],
  dict["oNe" => 1],
  dict["one" => 1, "two" => 2],
  dict["ONE" => 1, "two" => 2],
  dict["OnE" => 1, "two" => 2],
  dict["oNe" => 1, "two" => 2],
  dict["one" => 1, "TWO" => 2],
  dict["ONE" => 1, "TWO" => 2],
  dict["OnE" => 1, "TWO" => 2],
  dict["oNe" => 1, "TWO" => 2],
  dict["one" => 1, "TwO" => 2],
  dict["ONE" => 1, "TwO" => 2],
  dict["OnE" => 1, "TwO" => 2],
  dict["oNe" => 1, "TwO" => 2],
  dict["one" => 1, "tWo" => 2],
  dict["ONE" => 1, "tWo" => 2],
  dict["OnE" => 1, "tWo" => 2],
  dict["oNe" => 1, "tWo" => 2],
  dict["one" => 1, 0 => 2],
  dict["ONE" => 1, 0 => 2],
  dict["OnE" => 1, 0 => 2],
  dict["oNe" => 1, 0 => 2],
  dict["ONE" => 1, "TWO" => 2, "THREE" => 3, "FOUR" => "four"],
  dict["one" => 1, "two" => 2, "three" => 3, "four" => "FOUR"],
  dict["ONE" => 1, "TWO" => 2, "three" => 3, "four" => "FOUR"],
  dict["one" => 1, "two" => 2, "THREE" => 3, "FOUR" => "four"]
];

echo "*** Testing basic operations ***\n";
$loop_counter = 1;
foreach ($arrays as $item) {
        echo "** Iteration $loop_counter **\n"; $loop_counter++;
    var_dump(array_change_key_case($item));
    var_dump(array_change_key_case($item, CASE_UPPER));
    var_dump(array_change_key_case($item, CASE_LOWER));
    echo "\n";
}

echo "end\n";
}
