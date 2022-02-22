<?hh

function foo(): int {
  echo "bla\n";
  return 42;
}

// defaults
class C {
  public function __construct() {
    echo "C\n";
  }
}

// pure
class D {
  public function __construct()[] {
  }
}

// XParamDefinition lookalike
class F {
  private int $x;
  public function __construct()[] {
    $this->x = 42;
  }

  public function set(int $x)[write_props] : this {
    //  public function set(int $x) : this { // No ok, default is not compatible
    $this->x = $x;
    return $this;
  }
}

// XParam factory lookalike
abstract class Factory {
  public static function get()[] : F {
    return new F();
  }
}

// enum class contexts is write_props
enum class E : mixed {
   int X = 3;                   // ok
   string Y = "zuck";           // ok
   D Z1 = new D();              // ok
   F Z2 = Factory::get()->set(1664); // ok
}
