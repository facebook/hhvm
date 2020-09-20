<?hh
/* Prototype: array array_change_key_case ( array $input [, int $case] )
   Description: Changes the keys in the input array to be all lowercase
   or uppercase. The change depends on the last optional case parameter.
   You can pass two constants there, CASE_UPPER and CASE_LOWER(default).
   The function will leave number indices as is.
*/
<<__EntryPoint>> function main(): void {
$arrays = varray [
  varray [],
  varray [0],
  varray [1],
  varray [-1],
  varray [0, 2, 3, 4, 5],
  varray [1, 2, 3, 4, 5],
  darray ["" => 1],
  darray ["a" => 1],
  darray ["Z" => 1],
  darray ["one" => 1],
  darray ["ONE" => 1],
  darray ["OnE" => 1],
  darray ["oNe" => 1],
  darray ["one" => 1, "two" => 2],
  darray ["ONE" => 1, "two" => 2],
  darray ["OnE" => 1, "two" => 2],
  darray ["oNe" => 1, "two" => 2],
  darray ["one" => 1, "TWO" => 2],
  darray ["ONE" => 1, "TWO" => 2],
  darray ["OnE" => 1, "TWO" => 2],
  darray ["oNe" => 1, "TWO" => 2],
  darray ["one" => 1, "TwO" => 2],
  darray ["ONE" => 1, "TwO" => 2],
  darray ["OnE" => 1, "TwO" => 2],
  darray ["oNe" => 1, "TwO" => 2],
  darray ["one" => 1, "tWo" => 2],
  darray ["ONE" => 1, "tWo" => 2],
  darray ["OnE" => 1, "tWo" => 2],
  darray ["oNe" => 1, "tWo" => 2],
  darray ["one" => 1, 0 => 2],
  darray ["ONE" => 1, 0 => 2],
  darray ["OnE" => 1, 0 => 2],
  darray ["oNe" => 1, 0 => 2],
  darray ["ONE" => 1, "TWO" => 2, "THREE" => 3, "FOUR" => "four"],
  darray ["one" => 1, "two" => 2, "three" => 3, "four" => "FOUR"],
  darray ["ONE" => 1, "TWO" => 2, "three" => 3, "four" => "FOUR"],
  darray ["one" => 1, "two" => 2, "THREE" => 3, "FOUR" => "four"]
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
