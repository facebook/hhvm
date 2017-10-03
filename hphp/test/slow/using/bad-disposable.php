<?hh

function main() {
  echo "Entering main\n";

  echo "5\n";
  try {
    using (5) {}
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  echo "replace in using\n";
  try {
    using ($x = new stdclass) {
      $x = 12.0;
    }
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  echo "unset in using\n";
  $x = new stdclass;
  try {
    using ($x) {
      unset($x);
    }
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  echo "Leaving main\n\n";
}

async function mainAsync() {
  echo "\nEntering mainAsync\n";

  try {
    await using (5) {}
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  echo "Leaving mainAsync\n";
}

main();
HH\Asio\join(mainAsync());
