<?hh

<<__EntryPoint>>
function main_global_document() :mixed{
  // Make the test runner happy. It expects this endpoint to exist.
  if ($_SERVER['REQUEST_URI'] == '/hello.php') {
    $testid = getenv('TESTID');
    echo "Hello, World!{$testid}";
    return;
  }

  echo "Hit global document\n";
}
