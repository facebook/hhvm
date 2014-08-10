<?hh

function main($val) {
  $msarray = msarray();
  $msarray["foo"] = "bar";
  var_dump($msarray);

  $msarray = msarray(
    "foo" => 1,
    "bar" => 2,
    "baz" => false,
  );
  var_dump($msarray);

  $msarray = msarray(
    10 => $val,
  );
  var_dump($msarray);
}

main(true);
