<?hh

async function genMSArray() {
  return msarray(
    'key' => 'value',
  );
}

async function main() {
  $foo = await genMSArray();
  $foo[10] = 'warning';
  var_dump($foo);
}

main();
