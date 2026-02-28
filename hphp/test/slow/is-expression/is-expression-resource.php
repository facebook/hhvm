<?hh

function main(): void {
  echo "Opening...\n";
  $res = fopen(__FILE__, 'r');

  echo gettype($res)."\n";
  if ($res is resource) {
    echo "resource\n";
  } else {
    echo "not resource\n";
  }

  echo "Closing...\n";
  fclose($res);

  echo gettype($res)."\n";
  if ($res is resource) {
    echo "resource\n";
  } else {
    echo "not resource\n";
  }
}


<<__EntryPoint>>
function main_is_expression_resource() :mixed{
main();
}
