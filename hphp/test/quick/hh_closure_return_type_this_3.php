<?hh


function handler($errno, $errmsg) :mixed{
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

type my_t = int;

function f1() :mixed{ return function ($p) { return $p; }; }
function f2() :mixed{ return function ($p): int { return $p; }; }
function f2_soft() :mixed{ return function ($p): <<__Soft>> int { return $p; }; }
function f3() :mixed{ return function ($p): string { return $p; }; }
function f3_soft() :mixed{ return function ($p): <<__Soft>> string { return $p; }; }
function f4() :mixed{ return function ($p): bool { return $p; }; }
function f4_soft() :mixed{ return function ($p): <<__Soft>> bool { return $p; }; }
function f5() :mixed{ return function ($p): float { return $p; }; }
function f5_soft() :mixed{ return function ($p): <<__Soft>> float { return $p; }; }
function f6() :mixed{ return function ($p): resource { return $p; }; }
function f6_soft() :mixed{ return function ($p): <<__Soft>> resource { return $p; }; }
function f7() :mixed{ return function ($p): varray { return $p; }; }
function f7_soft() :mixed{ return function ($p): <<__Soft>> varray { return $p; }; }
function f8() :mixed{ return function ($p): callable { return $p; }; }
function f8_soft() :mixed{ return function ($p): <<__Soft>> callable { return $p; }; }
function f9() :mixed{ return function ($p): Figure { return $p; }; }
function f9_soft() :mixed{ return function ($p): <<__Soft>> Figure { return $p; }; }
function f10() :mixed{ return function ($p): Square { return $p; }; }
function f10_soft() :mixed{ return function ($p): <<__Soft>> Square { return $p; }; }
function f11() :mixed{ return function ($p): :div { return $p; }; }
function f11_soft() :mixed{ return function ($p): <<__Soft>> :div { return $p; }; }
function f12() :mixed{ return function ($p): Fractal<Square> { return $p; }; }
function f12_soft() :mixed{ return function ($p): <<__Soft>> Fractal<Square> { return $p; }; }
function f13<T>() :mixed{ return function ($p): Fractal<T> { return $p; }; }
function f13_soft<T>() :mixed{ return function ($p): <<__Soft>> Fractal<T> { return $p; }; }
function f14() :mixed{ return function ($p): my_t { return $p; }; }
function f14_soft() :mixed{ return function ($p): <<__Soft>> my_t { return $p; }; }
function f15() :mixed{ return function ($p): void { return $p; }; }
function f15_soft() :mixed{ return function ($p): <<__Soft>> void { return $p; }; }
function f16() :mixed{ return function ($p): mixed { return $p; }; }
function f16_soft() :mixed{ return function ($p): <<__Soft>> mixed { return $p; }; }
function f17() :mixed{ return function ($p): ?int { return $p; }; }
function f17_soft() :mixed{ return function ($p): <<__Soft>> ?int { return $p; }; }
function f18() :mixed{ return function ($p): (string, int) { return $p; }; }
function f18_soft() :mixed{ return function ($p): <<__Soft>> (string, int) { return $p; }; }
function f19() :mixed{ return function ($p): (function(int): int) { return $p; }; }
function f19_soft() :mixed{
  return function ($p): <<__Soft>> (function(int): int) { return $p; };
}
function f20() :mixed{ return function ($p): callable { return $p; }; }
function f20_soft() :mixed{ return function ($p): <<__Soft>> callable { return $p; }; }

class Figure {}
class Square extends Figure {}
class Fractal<T> extends Figure {}
class :div {}

class A {}
class B extends A {
  public function f21() :mixed{ return function (): this { return $this; }; }
  public function f21_soft() :mixed{ return function (): <<__Soft>> this { return $this; }; }
  public function f22() :mixed{ return function ($p): this { return $p; }; }
  public function f22_soft() :mixed{ return function ($p): <<__Soft>> this { return $p; }; }
  public static function testfunc() :mixed{}
}
class C extends B {}

function testfunc() :mixed{}

function call_wrapper($fn, $arg) :mixed{
  try {
    $fn($arg);
  } catch (Exception $e) {
    echo "Caught exception: " . $e->getMessage() . "\n";
  }
}

<<__EntryPoint>> function main(): void {
  error_reporting(-1);
  set_error_handler(handler<>);
  for ($i = 1; $i <= 20; $i++) {
    foreach (vec['', '_soft'] as $suffix) {
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
      call_wrapper($f(), vec[]);
      call_wrapper($f(), function($x){return $x*$x;});
      call_wrapper($f(), new Figure());
      call_wrapper($f(), new Square());
      call_wrapper($f(), new Fractal());
      call_wrapper($f(), <div/>);
      call_wrapper($f(), 'testfunc');
      call_wrapper($f(), vec['C', 'testfunc']);
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
    'f22' => vec[$c, 'f22'],
    'f22_soft' => vec[$c, 'f22_soft'],
  };
  foreach ($callbacks as $name => $f) {
    echo "\ncalling $name\n";
    call_wrapper($f(), null);
    call_wrapper($f(), 42);
    call_wrapper($f(), 'foobar');
    call_wrapper($f(), true);
    call_wrapper($f(), 14.1);
    call_wrapper($f(), imagecreate(10, 10));
    call_wrapper($f(), vec[]);
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
