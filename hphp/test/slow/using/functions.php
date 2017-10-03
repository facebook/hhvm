<?hh

require 'logger.inc';

function main() {
  echo "Entering main()\n";
  using new Logger();

  $f = function() {
    echo "Entering function\n";
    using new Logger();
    echo "Leaving function\n";
  };
  $f();

  echo "Adding another Logger\n";
  using new Logger();

  $f = () ==> {
    echo "Entering lambda\n";
    using new Logger();
    echo "Leaving lambda\n";
  };
  $f();
  $f();

  echo "Adding more Loggers\n";
  using new Logger();
  using (new Logger());

  echo "Done with main\n";
}

async function mainAsync() {
  echo "Entering mainAsync()\n";
  await using new Logger();
  await using await Logger::makeAsync();
  using await Logger::makeAsync();
  echo "Leaving mainAsync()\n";
}

function thrower() {
  echo "Entering thrower()\n";
  using new Logger();
  using new Logger();
  echo "About to throw\n";
  throw new Exception('hi');
}

class Foo {
  static function bar() {
    echo "Entering Foo::bar()\n";
    using new Logger();
    echo "Leaving Foo::bar()\n";
  }
}

main();
echo "Returned from main\n";
HH\Asio\join(mainAsync());
Foo::bar();

try {
  thrower();
} catch (Exception $e) {
  var_dump($e->getMessage());
}
