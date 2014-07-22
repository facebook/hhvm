<?hh

function main($val) {
  $miarray = miarray();
  $miarray[10] = "foo";
  var_dump($miarray);

  $miarray = miarray(
    1 => "foo",
    -1 => "bar",
    200 => "baz",
  );
  var_dump($miarray);

  $miarray = miarray(
    "warning" => $val,
  );
  var_dump($miarray);
}

main(true);
