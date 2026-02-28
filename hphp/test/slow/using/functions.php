<?hh

function main() :mixed{
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

async function mainAsync() :Awaitable<mixed>{
  echo "Entering mainAsync()\n";
  await using new Logger();
  await using await Logger::makeAsync();
  using await Logger::makeAsync();
  echo "Leaving mainAsync()\n";
}

function thrower() :mixed{
  echo "Entering thrower()\n";
  using new Logger();
  using new Logger();
  echo "About to throw\n";
  throw new Exception('hi');
}

class Foo {
  static function bar() :mixed{
    echo "Entering Foo::bar()\n";
    using new Logger();
    echo "Leaving Foo::bar()\n";
  }
}


<<__EntryPoint>>
function main_functions() :mixed{
require 'logger.inc';

main();
echo "Returned from main\n";
HH\Asio\join(mainAsync());
Foo::bar();

try {
  thrower();
} catch (Exception $e) {
  var_dump($e->getMessage());
}
}
