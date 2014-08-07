<?hh

function main() {
  $miarray = miarray(
    1 => 2,
    2 => 3,
    4 => 5,
  );
  var_dump(array_diff($miarray, miarray(3 => 6), array(0), miarray(2=>2,)));
}

main();
