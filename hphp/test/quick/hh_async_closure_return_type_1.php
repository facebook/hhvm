<?hh


error_reporting(-1);
function handler($errno, $errmsg) {
  $pos = strpos($errmsg, ", Closure");
  if ($pos !== false) {
    $errmsg = substr($errmsg, 0, $pos) . ", Closure given";
  }
  if ($errno === E_RECOVERABLE_ERROR) {
    throw new Exception($errmsg);
  } else if ($errno === E_WARNING) {
    echo "Triggered E_WARNING: $errmsg\n";
  } else if ($errno === E_NOTICE) {
    echo "Triggered E_NOTICE: $errmsg\n";
  } else {
    return false;
  }
}
set_error_handler('handler');

type my_t = int;

function f1() { return async function ($p) { return $p; }; }
function f2() { return async function ($p): Awaitable<int> { return $p; }; }
function f2_soft() {
  return async function ($p): @Awaitable<int> { return $p; };
}
function f3() { return async function ($p): Awaitable<string> { return $p; }; }
function f3_soft() {
  return async function ($p): @Awaitable<string> { return $p; };
}
function f4() { return async function ($p): Awaitable<bool> { return $p; }; }
function f4_soft() {
  return async function ($p): @Awaitable<bool> { return $p; };
}
function f5() { return async function ($p): Awaitable<float> { return $p; }; }
function f5_soft() {
  return async function ($p): @Awaitable<float> { return $p; };
}
function f6() {
  return async function ($p): Awaitable<resource> { return $p; };
}
function f6_soft() {
  return async function ($p): @Awaitable<resource> { return $p; };
}
function f7() {
  return async function ($p): Awaitable<array> { return $p; };
}
function f7_soft() {
  return async function ($p): @Awaitable<array> { return $p; };
}
function f8() {
  return async function ($p): Awaitable<callable> { return $p; };
}
function f8_soft() {
  return async function ($p): @Awaitable<callable> { return $p; };
}
function f9() { return async function ($p): Awaitable<Figure> { return $p; }; }
function f9_soft() {
  return async function ($p): @Awaitable<Figure> { return $p; };
}
function f10() { return async function ($p): Awaitable<Square> { return $p; }; }
function f10_soft() {
  return async function ($p): @Awaitable<Square> { return $p; };
}
function f11() { return async function ($p): Awaitable<:div> { return $p; }; }
function f11_soft() {
  return async function ($p): @Awaitable<:div> { return $p; };
}
function f12() {
  return async function ($p): Awaitable<Fractal<Square>> { return $p; };
}
function f12_soft() {
  return async function ($p): @Awaitable<Fractal<Square>> { return $p; };
}
function f13<T>() {
  return async function ($p): Awaitable<Fractal<T>> { return $p; };
}
function f13_soft<T>() {
  return async function ($p): @Awaitable<Fractal<T>> { return $p; };
}
function f14() { return async function ($p): Awaitable<my_t> { return $p; }; }
function f14_soft() {
  return async function ($p): @Awaitable<my_t> { return $p; };
}
function f15() { return async function ($p): Awaitable<void> { return $p; }; }
function f15_soft() {
  return async function ($p): @Awaitable<void> { return $p; };
}
function f16() { return async function ($p): Awaitable<mixed> { return $p; }; }
function f16_soft() {
  return async function ($p): @Awaitable<mixed> { return $p; };
}
function f17() { return async function ($p): Awaitable<?int> { return $p; }; }
function f17_soft() {
  return async function ($p): @Awaitable<?int> { return $p; };
}
function f18() {
  return async function ($p): Awaitable<(string, int)> { return $p; };
}
function f18_soft() {
  return async function ($p): @Awaitable<(string, int)> { return $p; };
}
function f19() {
  return async function ($p): Awaitable<(function(int): int)> { return $p; };
}
function f19_soft() {
  return async function ($p): @Awaitable<(function(int): int)> { return $p; };
}
function f20() {
  return async function ($p): Awaitable<callable> { return $p; };
}
function f20_soft() {
  return async function ($p): @Awaitable<callable> { return $p; };
}

class Figure {}
class Square extends Figure {}
class Fractal<T> extends Figure {}
class :div {}

class A {}
class B extends A {
  public function f21() {
    return async function (): Awaitable<this> { return $this; };
  }
  public function f21_soft() {
    return async function (): @Awaitable<this> { return $this; };
  }
  public function f22() {
    return async function ($p): Awaitable<this> { return $p; };
  }
  public function f22_soft() {
    return async function ($p): @Awaitable<this> { return $p; };
  }
  public static function f23() {
    return async function ($p): Awaitable<self> { return $p; };
  }
  public static function f23_soft() {
    return async function ($p): @Awaitable<self> { return $p; };
  }
  public static function f24() {
    return async function ($p): Awaitable<parent> { return $p; };
  }
  public static function f24_soft() {
    return async function ($p): @Awaitable<parent> { return $p; };
  }
  public static function testfunc() {}
}
class C extends B {}

function testfunc() {}

function call_wrapper($fn, $arg) {
  try {
    HH\Asio\join($fn($arg));
  } catch (Exception $e) {
    echo "Caught exception: " . $e->getMessage() . "\n";
  }
}

function main() {
  for ($i = 1; $i <= 20; $i++) {
    foreach (array('', '_soft') as $suffix) {
      if ($suffix !== '' && $i === 1) {
        // f1_soft() does not exist
        continue;
      }
      $f = 'f' . $i . $suffix;
      echo "\ncalling $f\n";
      call_wrapper($f(), null);
      call_wrapper($f(), 42);
      call_wrapper($f(), 'foobar');
      call_wrapper($f(), true);
      call_wrapper($f(), 14.1);
      call_wrapper($f(), imagecreate(10, 10));
      call_wrapper($f(), array());
      call_wrapper($f(), function($x){return $x*$x;});
      call_wrapper($f(), new Figure());
      call_wrapper($f(), new Square());
      call_wrapper($f(), new Fractal());
      call_wrapper($f(), <div/>);
      call_wrapper($f(), 'testfunc');
      call_wrapper($f(), array('C', 'testfunc'));
    }
  }

  $c = new C();
  echo "\ncalling f21\n";
  try {
    $f = $c->f21();
    HH\Asio\join($f());
  } catch (Exception $e) {
    echo "Caught exception: " . $e->getMessage() . "\n";
  }
  try {
    $f = $c->f21_soft();
    HH\Asio\join($f());
  } catch (Exception $e) {
    echo "Caught exception: " . $e->getMessage() . "\n";
  }

  $callbacks = Map {
    'f22' => array($c, 'f22'),
    'f22_soft' => array($c, 'f22_soft'),
    'f23' => array('C', 'f23'),
    'f23_soft' => array('C', 'f23_soft'),
    'f24' => array('C', 'f24'),
    'f24_soft' => array('C', 'f24_soft')
  };
  foreach ($callbacks as $name => $f) {
    echo "\ncalling $name\n";
    call_wrapper($f(), null);
    call_wrapper($f(), 42);
    call_wrapper($f(), 'foobar');
    call_wrapper($f(), true);
    call_wrapper($f(), 14.1);
    call_wrapper($f(), imagecreate(10, 10));
    call_wrapper($f(), array());
    call_wrapper($f(), function($x){return $x*$x;});
    call_wrapper($f(), new Figure());
    call_wrapper($f(), new Square());
    call_wrapper($f(), new Fractal());
    call_wrapper($f(), <div/>);
    call_wrapper($f(), new A());
    call_wrapper($f(), new B());
    call_wrapper($f(), new C());
  }
  echo "Done\n";
}
main();
