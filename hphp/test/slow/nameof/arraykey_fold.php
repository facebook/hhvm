<?hh

namespace N {
  function f(): void {
    $d = dict["N\\C" => "N\\f -- N\\C", "C" => "N\\f -- C"];

    \var_dump($d["N\\C"]);
    \var_dump($d[C::class]);
    \var_dump($d[nameof C]);
    \var_dump($d[\N\C::class]);
    \var_dump($d[nameof \N\C]);

    \var_dump($d["C"]);
    \var_dump($d[\C::class]);
    \var_dump($d[nameof \C]);
  }
}
namespace {
  class B {
    public static function foo(): void {
      $d = dict["B" => "foo -- B", "C" => "foo -- C"];

      var_dump($d[self::class]);
      var_dump($d[nameof self]);
      var_dump($d[static::class]); // not folded
      var_dump($d[nameof static]); // not folded
    }
  }
  class C extends B {
    public static function bar(): void {
      $d = dict["B" => "bar -- B", "C" => "bar -- C"];

      var_dump($d[self::class]);
      var_dump($d[nameof self]);
      var_dump($d[parent::class]); // not folded
      var_dump($d[nameof parent]); // not folded
    }
  }

  function f(): void {
    $c = dict["C" => "f -- C"];

    var_dump($c["C"]);
    var_dump($c[C::class]);
    var_dump($c[nameof C]);
  }

  <<__EntryPoint>>
  function main(): void {
    f();
    echo "--------------------\n";
    N\f();
    echo "--------------------\n";
    B::foo();
    echo "--------------------\n";
    C::foo();
    echo "--------------------\n";
    C::bar();
  }
}
