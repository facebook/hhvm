<?hh

function decode($json) :mixed{
  $error = null;
  $result = json_decode_with_error($json, inout $error);
  var_dump($result);
  var_dump($error !== null);
  echo "\n";
}

<<__EntryPoint>>
function main(): void {
  // Leading whitespace should be ignored
  decode(" true");
  decode("\ttrue");
  decode("\ntrue");
  decode("\rtrue");

  // So should trailing whitespace
  decode("true ");
  decode("true\t");
  decode("true\n");
  decode("true\r");

  // And so should the combination of both
  decode(" true ");
  decode(" true\t");
  decode(" true\n");
  decode(" true\r");
  decode("\ttrue ");
  decode("\ttrue\t");
  decode("\ttrue\n");
  decode("\ttrue\r");
  decode("\ntrue ");
  decode("\ntrue\t");
  decode("\ntrue\n");
  decode("\ntrue\r");
  decode("\rtrue ");
  decode("\rtrue\t");
  decode("\rtrue\n");
  decode("\rtrue\r");

  echo "Done\n";
}
