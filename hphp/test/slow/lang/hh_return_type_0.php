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

function f1($p) { return $p; }
function f2($p): int { return $p; }
function f2_soft($p): @int { return $p; }
function f3($p): string { return $p; }
function f3_soft($p): @string { return $p; }
function f4($p): bool { return $p; }
function f4_soft($p): @bool { return $p; }
function f5($p): float { return $p; }
function f5_soft($p): @float { return $p; }
function f6($p): resource { return $p; }
function f6_soft($p): @resource { return $p; }
function f7($p): array { return $p; }
function f7_soft($p): @array { return $p; }
function f8($p): callable { return $p; }
function f8_soft($p): @callable { return $p; }
function f9($p): Figure { return $p; }
function f9_soft($p): @Figure { return $p; }
function f10($p): Square { return $p; }
function f10_soft($p): @Square { return $p; }
function f11($p): :div { return $p; }
function f11_soft($p): @:div { return $p; }
function f12($p): Fractal<Square> { return $p; }
function f12_soft($p): @Fractal<Square> { return $p; }
function f13<T>($p): Fractal<T> { return $p; }
function f13_soft<T>($p): @Fractal<T> { return $p; }
function f14($p): my_t { return $p; }
function f14_soft($p): @my_t { return $p; }
function f15($p): void { return $p; }
function f15_soft($p): @void { return $p; }
function f16($p): mixed { return $p; }
function f16_soft($p): @mixed { return $p; }
function f17($p): ?int { return $p; }
function f17_soft($p): @?int { return $p; }
function f18($p): (string, int) { return $p; }
function f18_soft($p): @(string, int) { return $p; }
function f19($p): (function(int): int) { return $p; }
function f19_soft($p): @(function(int): int) { return $p; }
function f20($p): callable { return $p; }
function f20_soft($p): @callable { return $p; }
function f21($p): noreturn { return $p; }
function f21_soft($p): @noreturn { return $p; }

class Figure {}
class Square extends Figure {}
class Fractal<T> extends Figure {}
class :div {}

class A {}
class B extends A {
  public function f21(): this { return $this; }
  public function f21_soft(): @this { return $this; }
  public function f22($p): this { return $p; }
  public function f22_soft($p): @this { return $p; }
  public static function f23($p): self { return $p; }
  public static function f23_soft($p): @self { return $p; }
  public static function f24($p): parent { return $p; }
  public static function f24_soft($p): @parent { return $p; }
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
  for ($i = 1; $i <= 21; $i++) {
    foreach (array('', '_soft') as $suffix) {
      if ($suffix !== '' && $i === 1) {
        // f1_soft() does not exist
        continue;
      }
      $f = 'f' . $i . $suffix;
      echo "\ncalling $f\n";
      call_wrapper($f, null);
      call_wrapper($f, 42);
      call_wrapper($f, 'foobar');
      call_wrapper($f, true);
      call_wrapper($f, 14.1);
      call_wrapper($f, imagecreate(10, 10));
      call_wrapper($f, array());
      call_wrapper($f, function($x){return $x*$x;});
      call_wrapper($f, new Figure());
      call_wrapper($f, new Square());
      call_wrapper($f, new Fractal());
      call_wrapper($f, <div/>);
      call_wrapper($f, 'testfunc');
      call_wrapper($f, array('C', 'testfunc'));
    }
  }

  $c = new C();
  echo "\ncalling f21\n";
  try {
    $c->f21();
  } catch (Exception $e) {
    echo "Caught exception: " . $e->getMessage() . "\n";
  }
  try {
    $c->f21_soft();
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
    call_wrapper($f, null);
    call_wrapper($f, 42);
    call_wrapper($f, 'foobar');
    call_wrapper($f, true);
    call_wrapper($f, 14.1);
    call_wrapper($f, imagecreate(10, 10));
    call_wrapper($f, array());
    call_wrapper($f, function($x){return $x*$x;});
    call_wrapper($f, new Figure());
    call_wrapper($f, new Square());
    call_wrapper($f, new Fractal());
    call_wrapper($f, <div/>);
    call_wrapper($f, new A());
    call_wrapper($f, new B());
    call_wrapper($f, new C());
  }
  echo "Done\n";
}
main();
