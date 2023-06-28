<?hh


class Base {
  public static function test() :mixed{
    static::clowntown();
  }
}

class Derived1 extends Base {
  public static function clowntown() :mixed{
    print "Welcome to Derived1::clowntown()\n";
  }
}

class Derived2 extends Base {
  public static function clowntown() :mixed{
    print "Welcome to Derived2::clowntown()\n";
  }
}

function main() :mixed{
  Derived1::test();
  Derived2::test();
}


<<__EntryPoint>>
function main_undef_static_call() :mixed{
main();
}
