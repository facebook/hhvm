<?hh

function main() {
  $miarr = miarray(
    1 => 2
  );
  unset($miarr['warn']);
  var_dump($miarr);
  unset($miarr['still warn']);
  var_dump($miarr);
}

main();
