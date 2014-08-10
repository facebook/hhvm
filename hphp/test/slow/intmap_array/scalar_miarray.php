<?hh

function foo() {
  return miarray(
    1 => 2,
    3 => 'foo',
  );
}

function main() {
  $miarray = foo();
  $miarray['foo'] = "warning";
  var_dump($miarray);
}

main();
