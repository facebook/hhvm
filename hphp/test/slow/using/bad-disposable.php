<?hh

function main() :mixed{
  echo "Entering main\n";

  echo "5\n";
  try {
    using (5) {}
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  echo "replace in using\n";
  try {
    using ($x = new stdClass) {
      $x = 12.0;
    }
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  echo "unset in using\n";
  $x = new stdClass;
  try {
    using ($x) {
      unset($x);
    }
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  echo "Leaving main\n\n";
}

async function mainAsync() :Awaitable<mixed>{
  echo "\nEntering mainAsync\n";

  try {
    await using (5) {}
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  echo "Leaving mainAsync\n";
}


<<__EntryPoint>>
function main_bad_disposable() :mixed{
main();
HH\Asio\join(mainAsync());
}
