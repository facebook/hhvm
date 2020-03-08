<?hh // strict

namespace NS_dynamic_methods;

class Widget {
  public function __call(string $name, array<mixed> $arguments): int {
    echo "Calling instance method >$name<\n";
    var_dump($arguments);

    return 987;
  }

  public static function __callStatic(string $name, array<mixed> $arguments): string {
    echo "Calling static method >$name<\n";
    var_dump($arguments);

    return "hello";
  }
}

function main(): void {
  $obj = new Widget();

  $obj->iMethod(10, true, "abc");
//  $obj->__call('iMethod', array(10, true, "abc"));
//  $obj->__call('123#$%', []);

  Widget::sMethod(null, 1.234);
  Widget::__callStatic('sMethod', array(null, 1.234));
  Widget::__callStatic('[]{}', []);
}

/* HH_FIXME[1002] call to main in strict*/
main();
