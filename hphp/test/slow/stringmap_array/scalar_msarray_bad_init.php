<?hh

function foo() {
  return msarray(
    'one' => 1,
    2 => 2,
  );
}

function main() {
  $msarray = foo();
  var_dump($msarray);
}

main();
