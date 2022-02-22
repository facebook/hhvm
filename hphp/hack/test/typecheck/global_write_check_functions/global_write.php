<?hh // strict

class Bar {
  public int $prop = 0;

  public function get_prop(): int {
    return $this->prop;
  }

  public function set_prop(int $i): void {
    $this->prop = $i;
  }
}

class Foo {
  public static int $static_prop = 1;
  <<__LateInit>> public static Bar $bar;
  public static vec<int> $static_vec_int = vec[0, 1, 2];
  <<__LateInit>> public static vec<Bar> $static_vec_bar;

  public static function set_bar(): void {
    self::$bar = new Bar(); // A global variable is written.
  }

  public static function set_vec_bar(): void {
    self::$static_vec_bar = vec[new Bar()]; // A global variable is written.
  }

  <<__MemoizeLSB>>
  public static function memoized_lsb(): Bar {
    return self::$bar; // A global variable is passed to (or returned from) a function call.
  }
}

<<__Memoize>>
function memoized(): Bar {
  return new Bar();
}

<<__Memoize>>
function memoized_vec_int(): vec<int> {
  return vec[1, 2, 3];
}

function test_fun_call(): Bar {
  $a = Foo::$bar;
  $a->prop = 2; // A global variable is written.
  return $a; // A global variable is passed to (or returned from) a function call.
}

class Test {
  public function test_method_call(): void {
    Foo::set_bar();
    Foo::set_vec_bar();

    Foo::$static_prop = 2; // A global variable is written.
    Foo::$static_prop += 1; // A global variable is written.
    Foo::$static_prop++; // A global variable is written.

    (Foo::$bar)->prop = 2; // A global variable is written.
    Foo::$bar = new Bar(); // A global variable is written.

    $a = Foo::$static_prop;
    $a = 2;

    $b = Foo::$bar;
    $b->prop = 2; // A global variable is written.
    $b->set_prop(2); // To do: how to detect such a global write???
    $b->prop++; // A global variable is written.

    $b = $this->call_bar($b); // A global variable is passed to (or returned from) a function call.
    $b->prop += 2; // Not sure if $b is global or not

    $c = $b->prop;
    $c = 3;

    $b = new Bar();
    $b->prop = 2;

    $b = Foo::$bar;
    $fun = (int $val): void ==> {
      Foo::$static_prop = 2; // A global variable is written.
      $b->prop = $val; // A global variable is written.
      $b = new Bar();
      $b->prop = $val;
    };
    $b->prop = 2; // A global variable is written.

    $d = Foo::memoized_lsb(); // A memoized function is called.
    $d->prop = 4;

    $e = memoized(); // A memoized function is called.
    $e->prop = 4;

    $f = Foo::$static_vec_int;
    $f[0] = 1;

    $g = Foo::$static_vec_bar;
    $g[0]->prop = 1; // A global variable is written.
    $g[0] = new Bar();
    $g[0]->prop = 1;

    $h = memoized_vec_int();
    $h[0] = 1;

    $x = new Bar();
    if (false) {$x = Foo::$bar;}
    $x->prop = 5; // A global variable is written.

    $y = Foo::$bar;
    if (false) {$y = new Bar();}
    $y->prop = 5; // To do: merge flows and expect an error here
  }

  public function call_bar(Bar $x): Bar {
    $x = new Bar();
    return $x;
  }
}
