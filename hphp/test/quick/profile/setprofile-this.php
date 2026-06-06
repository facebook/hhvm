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
  public function method() :mixed{
    echo "\nC method\n";
    return new A;
  }
}

abstract final class ProfilerStatics {
  public static $indent = 2;
}

function profiler($event, $name, $info) :mixed{
  if ($name == 'get_class') return;

  if ($event == 'exit') --ProfilerStatics::$indent;
  printf("\n%s%s %s: %s\n", str_repeat('  ', ProfilerStatics::$indent), $event,
         $name, serialize($info));
  if ($event == 'enter') ++ProfilerStatics::$indent;
}

<<__EntryPoint>>
function setprofile_this(): void {
  fb_setprofile(profiler<>);
  new C();
  (new C())->method();
}
