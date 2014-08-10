<?hh

function foo() {
  return miarray(
    1 => 2,
    'two' => 3,
  );
}

function main() {
  $miarray = foo();
  var_dump($miarray);
}

main();
