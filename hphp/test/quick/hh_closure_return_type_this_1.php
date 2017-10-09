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

function f1() { return function ($p) { return $p; }; }
function f2() { return function ($p): int { return $p; }; }
function f2_soft() { return function ($p): @int { return $p; }; }
function f3() { return function ($p): string { return $p; }; }
function f3_soft() { return function ($p): @string { return $p; }; }
function f4() { return function ($p): bool { return $p; }; }
function f4_soft() { return function ($p): @bool { return $p; }; }
function f5() { return function ($p): float { return $p; }; }
function f5_soft() { return function ($p): @float { return $p; }; }
function f6() { return function ($p): resource { return $p; }; }
function f6_soft() { return function ($p): @resource { return $p; }; }
function f7() { return function ($p): array { return $p; }; }
function f7_soft() { return function ($p): @array { return $p; }; }
function f8() { return function ($p): callable { return $p; }; }
function f8_soft() { return function ($p): @callable { return $p; }; }
function f9() { return function ($p): Figure { return $p; }; }
function f9_soft() { return function ($p): @Figure { return $p; }; }
function f10() { return function ($p): Square { return $p; }; }
function f10_soft() { return function ($p): @Square { return $p; }; }
function f11() { return function ($p): :div { return $p; }; }
function f11_soft() { return function ($p): @:div { return $p; }; }
function f12() { return function ($p): Fractal<Square> { return $p; }; }
function f12_soft() { return function ($p): @Fractal<Square> { return $p; }; }
function f13<T>() { return function ($p): Fractal<T> { return $p; }; }
function f13_soft<T>() { return function ($p): @Fractal<T> { return $p; }; }
function f14() { return function ($p): my_t { return $p; }; }
function f14_soft() { return function ($p): @my_t { return $p; }; }
function f15() { return function ($p): void { return $p; }; }
function f15_soft() { return function ($p): @void { return $p; }; }
function f16() { return function ($p): mixed { return $p; }; }
function f16_soft() { return function ($p): @mixed { return $p; }; }
function f17() { return function ($p): ?int { return $p; }; }
function f17_soft() { return function ($p): @?int { return $p; }; }
function f18() { return function ($p): (string, int) { return $p; }; }
function f18_soft() { return function ($p): @(string, int) { return $p; }; }
function f19() { return function ($p): (function(int): int) { return $p; }; }
function f19_soft() {
  return function ($p): @(function(int): int) { return $p; };
}
function f20() { return function ($p): callable { return $p; }; }
function f20_soft() { return function ($p): @callable { return $p; }; }

class Figure {}
class Square extends Figure {}
class Fractal<T> extends Figure {}
class :div {}

class A {}
class B extends A {
  public function f21() { return function (): this { return $this; }; }
  public function f21_soft() { return function (): @this { return $this; }; }
  public function f22() { return function ($p): this { return $p; }; }
  public function f22_soft() { return function ($p): @this { return $p; }; }
  public static function f23() { return function ($p): self { return $p; }; }
  public static function f23_soft() {
    return function ($p): @self { return $p; };
  }
  public static function f24() { return function ($p): parent { return $p; }; }
  public static function f24_soft() {
    return function ($p): @parent { return $p; };
  }
  public static function testfunc() {}
}
class C extends B {}

function testfunc() {}

function call_wrapper($fn, $arg) {
  try {
    $fn($arg);
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
    $f();
  } catch (Exception $e) {
    echo "Caught exception: " . $e->getMessage() . "\n";
  }
  try {
    $f = $c->f21_soft();
    $f();
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
