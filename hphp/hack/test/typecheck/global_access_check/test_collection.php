<?hh

class Bar {
  public int $prop = 0;
}

class Foo {
  <<__LateInit>> public static Bar $static_bar;

  public static vec<int> $static_vec_int = vec[0, 1, 2];
  <<__LateInit>> public static vec<Bar> $static_vec_bar;

  public static function get_bar(): Bar {
    return self::$static_bar; // A global variable is passed to (or returned from) a function call.
  }

  public static function set_bar(Bar $b): void {
    self::$static_bar = $b; // A global variable is written.
  }

  public static function get_vec_int(): vec<int> {
    return self::$static_vec_int;
  }

  public static function set_vec_int(vec<int> $vi): void {
    self::$static_vec_int = $vi; // A global variable is written.
  }

  public static function get_vec_bar(): vec<Bar> {
    return self::$static_vec_bar; // A global variable is passed to (or returned from) a function call.
  }

  public static function set_vec_bar(vec<Bar> $vb): void {
    self::$static_vec_bar = $vb; // A global variable is written.
  }
}

class Test {
  public function test_method_call(): void {
    Foo::$static_bar = new Bar(); // A global variable is written.
    Foo::set_bar(new Bar());
    $b = Foo::$static_bar;
    $b->prop = 2; // A global variable is written.

    Foo::$static_vec_int = vec[3, 4, 5]; // A global variable is written.
    Foo::set_vec_int(vec[3, 4, 5]);
    $c = Foo::$static_vec_int;
    $c[0] = 0;
    $c = vec[3, 4, 5];
    Foo::$static_vec_int[0] = 0;

    Foo::$static_vec_bar = vec[new Bar()]; // A global variable is written.
    Foo::set_vec_bar(vec[new Bar()]);
    $d = Foo::$static_vec_bar;
    $d[0]->prop = 2; // A global variable is written.
    $d = vec[new Bar()];
    $d[0]->prop = 2;
    Foo::$static_vec_bar[0]->prop = 2; // A global variable is written.

    $e = shape('id' => 1, 'object' => Foo::$static_bar);
    $e['object'] = Foo::$static_bar;
    $e['object']->prop = 2; // A global variable is written.
    $e['id'] = 2;
    $e['object']->prop = 2; // To do: raise an error "A global variable is written."

    $f = vec[new Bar()];
    $f[] = Foo::$static_bar;
    $f[1]->prop = 2; // A global variable is written.
    $f[0] = new Bar();
    $f[1]->prop = 2; // To do: raise an error "A global variable is written."
  }
}
