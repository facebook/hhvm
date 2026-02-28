<?hh
<<__EntryPoint>> function main(): void {
echo "\n*** Testing array_keys() on various arrays ***";
$arrays = vec[
  vec[],
  vec[0],
  vec[ vec[] ],
  dict["Hello" => "World"],
  dict["" => ""],
  dict[0 => 1, 1 => 2, 2 => 3, "d" => vec[4,6, "d"]],
  dict["a" => 1, "b" => 2, "c" =>3, "d" => vec[]],
  dict[0 => 0, 1 => 1, 2 => 2, 3 => 3],
  dict[0=>3.000, 1=>2, 1=>3, "a"=>3, 3=>5, "5"=>3.000],
  dict[1 => TRUE, 0 => FALSE, '' => NULL, 2 => "\x000", 3 => "\000"],
  dict["a" => "abcd", "a" => "", "ab" => -6, "cd" => -0.5 ],
  dict[0 => vec[], 1=> vec[0], 2 => vec[1], ""=> vec[],""=>"" ]
];

$i = 0;
/* loop through to test array_keys() with different arrays */
foreach ($arrays as $array) {
  echo "\n-- Iteration $i --\n";
  var_dump(array_keys($array));
  $i++;
}

echo "Done\n";
}
