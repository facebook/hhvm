<?hh // strict

class Bar {
  public int $prop = 0;
}

class Foo {
  public static int $static_prop = 1;
  public static ?Bar $nullable_bar = null;
  <<__LateInit>> public static Bar $bar;
  public static dict<string, int> $static_dict = dict[];
}

function call_mixed(mixed $x): mixed {
  return 0;
}

class Test {
  public function test_method_call(): void {
    Foo::$static_prop = 2; // WriteLiteral
    Foo::$static_prop += 2; // WriteLiteral
    Foo::$static_prop++; // CounterIncrement

    $a = Foo::$nullable_bar;
    if ($a is null) {
      Foo::$nullable_bar = new Bar(); // Singleton
    } else {
      $a->prop = 1; // WriteLiteral
    }

    Foo::$nullable_bar = null; // WriteEmptyOrNUll

    $b = Foo::$bar;
    $c = $b->prop;
    Foo::$static_prop = $c; // WriteGlobalToGlobal

    $b->prop = (int)\microtime(true); // WriteNonSensitive
    $b->prop = Time::nowInMS(); // WriteNonSensitive
    $b->prop = SV_MESSAGING_CONFIG::getWithMarkedArrays_LEGACY(); // WriteNonSensitive
    Foo::$static_prop = $b->prop; // WriteGlobalToGlobal, WriteNonSensitive

    $b->prop = (int)\microtime(true) + Foo::$static_prop; // WriteNonSensitive, WriteGlobalToGlobal
    $b->prop = (int)\microtime(true) + Foo::$static_prop + (int)call_mixed(0); // NoPattern (call_mixed is unknown)

    Foo::$static_dict = HH\Lib\Dict\merge(Foo::$static_dict, dict["0" => 0]); // WriteLiteral,WriteGlobalToGlobal
    Foo::$static_dict = HH\Lib\Dict\merge(Foo::$static_dict, dict["0" => (int)call_mixed(0)]); // NoPattern (call_mixed is unknown)
    Foo::$static_dict = dict[]; // WriteEmptyOrNUll
    Foo::$static_dict["1"] ??= 1; // Caching
  }
}
