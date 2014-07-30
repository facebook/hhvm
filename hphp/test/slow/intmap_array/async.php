<?hh

async function genMIArray() {
  return miarray(
    1 => 'foo',
  );
}

async function main() {
  $foo = await genMIArray();
  $foo['warning'] = true;
  var_dump($foo);
}

main();
