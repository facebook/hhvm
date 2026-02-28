<?hh

function thrower() :mixed{
  throw new Exception('hi');
}

function main() :mixed{
  echo "Starting main\n";
  using (new Logger()) {
    echo "In first using\n";
  }

  echo "Empty using\n";
  using (new Logger()) {
  }

  using ($l = new Logger()) {
    echo "In second using\n";
    var_dump($l);
  }
  var_dump(isset($l));

  $l = new Logger();
  using ($l) {
    echo "In third using\n";
  }
  var_dump(isset($l));

  $l = new Logger();
  using ($l) {
    echo "Replacing using variable\n";
    $l = new Logger();
  }

  echo "Entering using with null variable\n";
  $l = null;
  using ($l) {
    $l = new Logger();
  }

  using (new Logger(), new Logger(), new Logger()) {
    echo "Triple using!\n";
    echo "Triple using second line\n";
  }

  using (new Logger()) {
    echo "Nested using\n";
    using (new Logger()) {
      echo "Inner using\n";
    }
  }

  try {
    echo "Trying throwing Logger\n";
    using (new Logger(), new Logger(true), new Logger()) {
      echo "Shouldn't get here\n";
    }
  } catch (Exception $e) {
    printf("Caught exception %s\n", $e->getMessage());
  }

  try {
    echo "Throwing inside using\n";
    using (new Logger()) {
      echo "About to throw";
      thrower();
    }
  } catch (Exception $e) {
    printf("Caught exception %s\n", $e->getMessage());
  }

  echo "Leaving main\n\n";
}

async function mainAsync() :Awaitable<mixed>{
  echo "\nStarting mainAsync\n";
  await using (new Logger()) {
    echo "Sync create, async dispose\n";
  }

  await using (await Logger::makeAsync()) {
    echo "Async both\n";
  }

  using (await Logger::makeAsync()) {
    echo "Async create, sync dispose\n";
  }

  using ($x = await Logger::makeAsync()) {
    echo "Async create, sync dispose, variable\n";
    var_dump($x);
  }
  var_dump(isset($x));

  using (new Logger(), await Logger::makeAsync()) {
    echo "Mixed create, sync dispose\n";
  }

  try {
    using (await Logger::makeAsync(), await Logger::makeAsync(true)) {
      echo "Shouldn't get here\n";
    }
  } catch (Exception $e) {
    printf("Caught exception %s\n", $e->getMessage());
  }

  echo "Leaving mainAsync\n\n";
}


<<__EntryPoint>>
function main_basic() :mixed{
require 'logger.inc';

main();
HH\Asio\join(mainAsync());

echo "In pseudomain\n";
using (new Logger()) {
  echo "Inside using\n";
}
echo "Outside using\n";
}
