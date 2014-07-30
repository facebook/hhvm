<?hh

function foo() {
  return msarray(
    'abc' => 2,
    'def' => 3,
  );
}

function main() {
  $msarray = foo();
  $msarray[1] = "warning";
  var_dump($msarray);
}

main();
