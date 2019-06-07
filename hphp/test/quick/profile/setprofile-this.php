<?hh
// Test uses RenameFunction to ensure that all arguments are retained

trait logger {
  public function __construct() {
    echo "\n".__CLASS__." constructing\n";
  }
}

class A {
  use logger;
}
class C {
  use logger;
  public function method() {
    echo "\nC method\n";
    return new A;
  }
}

abstract final class ProfilerStatics {
  public static $indent = 2;
  public static $threw = false;
}

function profiler($event, $name, $info) {
  if ($name == 'get_class') return;

  if ($event == 'exit') --ProfilerStatics::$indent;
  printf("\n%s%s %s: %s\n", str_repeat('  ', ProfilerStatics::$indent), $event,
         $name, serialize($info));
  if ($event == 'enter') ++ProfilerStatics::$indent;
  if ($event == 'exit' &&
      ((!ProfilerStatics::$threw && strncmp('C::', $name, 3) == 0) ||
       $name === 'C::method')) {
    ProfilerStatics::$threw = true;
    throw new Exception($name);
  }
}

fb_setprofile('profiler');

function main() {
  try {
    new C();
  } catch (Exception $e) {
    echo "\nCaught ".$e->getMessage()."\n";
  }

  try {
    (new C())->method();
  } catch (Exception $e) {
    echo "\nCaught ".$e->getMessage()."\n";
  }
}
main();
