<?hh

function main() {
  $msarray = msarray(
    'a' => 2,
    'b' => 3,
    'c' => 5,
  );
  var_dump(array_diff($msarray, msarray('a' => 6), array(0), msarray('b'=>2,)));
}

main();
