<?hh
<<__EntryPoint>> function main(): void {
echo "\n*** Testing array_keys() on various arrays ***";
$arrays = varray[
  varray[],
  varray[0],
  varray[ varray[] ],
  darray["Hello" => "World"],
  darray["" => ""],
  darray[0 => 1, 1 => 2, 2 => 3, "d" => varray[4,6, "d"]],
  darray["a" => 1, "b" => 2, "c" =>3, "d" => varray[]],
  darray[0 => 0, 1 => 1, 2 => 2, 3 => 3],
  darray[0.001=>3.000, 1.002=>2, 1.999=>3, "a"=>3, 3=>5, "5"=>3.000],
  darray[TRUE => TRUE, FALSE => FALSE, NULL => NULL, 2 => "\x000", 3 => "\000"],
  darray["a" => "abcd", "a" => "", "ab" => -6, "cd" => -0.5 ],
  darray[0 => varray[], 1=> varray[0], 2 => varray[1], ""=> varray[],""=>"" ]
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
