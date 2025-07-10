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

<<__DynamicallyCallable>> function f1($p) :mixed{ return $p; }
<<__DynamicallyCallable>> function f2($p): int { return $p; }
<<__DynamicallyCallable>> function f2_soft($p): <<__Soft>> int { return $p; }
<<__DynamicallyCallable>> function f3($p): string { return $p; }
<<__DynamicallyCallable>> function f3_soft($p): <<__Soft>> string { return $p; }
<<__DynamicallyCallable>> function f4($p): bool { return $p; }
<<__DynamicallyCallable>> function f4_soft($p): <<__Soft>> bool { return $p; }
<<__DynamicallyCallable>> function f5($p): float { return $p; }
<<__DynamicallyCallable>> function f5_soft($p): <<__Soft>> float { return $p; }
<<__DynamicallyCallable>> function f6($p): resource { return $p; }
<<__DynamicallyCallable>> function f6_soft($p): <<__Soft>> resource { return $p; }
<<__DynamicallyCallable>> function f7($p): varray { return $p; }
<<__DynamicallyCallable>> function f7_soft($p): <<__Soft>> varray { return $p; }
<<__DynamicallyCallable>> function f8($p): callable { return $p; }
<<__DynamicallyCallable>> function f8_soft($p): <<__Soft>> callable { return $p; }
<<__DynamicallyCallable>> function f9($p): Figure { return $p; }
<<__DynamicallyCallable>> function f9_soft($p): <<__Soft>> Figure { return $p; }
<<__DynamicallyCallable>> function f10($p): Square { return $p; }
<<__DynamicallyCallable>> function f10_soft($p): <<__Soft>> Square { return $p; }
<<__DynamicallyCallable>> function f11($p): :div { return $p; }
<<__DynamicallyCallable>> function f11_soft($p): <<__Soft>> :div { return $p; }
<<__DynamicallyCallable>> function f12($p): Fractal<Square> { return $p; }
<<__DynamicallyCallable>> function f12_soft($p): <<__Soft>> Fractal<Square> { return $p; }
<<__DynamicallyCallable>> function f13<T>($p): Fractal<T> { return $p; }
<<__DynamicallyCallable>> function f13_soft<T>($p): <<__Soft>> Fractal<T> { return $p; }
<<__DynamicallyCallable>> function f14($p): my_t { return $p; }
<<__DynamicallyCallable>> function f14_soft($p): <<__Soft>> my_t { return $p; }
<<__DynamicallyCallable>> function f15($p): void { return $p; }
<<__DynamicallyCallable>> function f15_soft($p): <<__Soft>> void { return $p; }
<<__DynamicallyCallable>> function f16($p): mixed { return $p; }
<<__DynamicallyCallable>> function f16_soft($p): <<__Soft>> mixed { return $p; }
<<__DynamicallyCallable>> function f17($p): ?int { return $p; }
<<__DynamicallyCallable>> function f17_soft($p): <<__Soft>> ?int { return $p; }
<<__DynamicallyCallable>> function f18($p): (string, int) { return $p; }
<<__DynamicallyCallable>> function f18_soft($p): <<__Soft>> (string, int) { return $p; }
<<__DynamicallyCallable>> function f19($p): (function(int): int) { return $p; }
<<__DynamicallyCallable>> function f19_soft($p): <<__Soft>> (function(int): int) { return $p; }
<<__DynamicallyCallable>> function f20($p): callable { return $p; }
<<__DynamicallyCallable>> function f20_soft($p): <<__Soft>> callable { return $p; }
<<__DynamicallyCallable>> function f21($p): noreturn { return $p; }
<<__DynamicallyCallable>> function f21_soft($p): <<__Soft>> noreturn { return $p; }

class Figure {}
class Square extends Figure {}
class Fractal<T> extends Figure {}
class :div {}

class A {}
class B extends A {
  public function f21(): this { return $this; }
  public function f21_soft(): <<__Soft>> this { return $this; }
  public function f22($p): this { return $p; }
  public function f22_soft($p): <<__Soft>> this { return $p; }
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

function main() :mixed{
  for ($i = 1; $i <= 21; $i++) {
    foreach (vec['', '_soft'] as $suffix) {
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
      call_wrapper($f, vec[]);
      call_wrapper($f, function($x){return $x*$x;});
      call_wrapper($f, new Figure());
      call_wrapper($f, new Square());
      call_wrapper($f, new Fractal());
      call_wrapper($f, <div/>);
      call_wrapper($f, 'testfunc');
      call_wrapper($f, vec['C', 'testfunc']);
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
    'f22' => vec[$c, 'f22'],
    'f22_soft' => vec[$c, 'f22_soft'],
  };
  foreach ($callbacks as $name => $f) {
    echo "\ncalling $name\n";
    call_wrapper($f, null);
    call_wrapper($f, 42);
    call_wrapper($f, 'foobar');
    call_wrapper($f, true);
    call_wrapper($f, 14.1);
    call_wrapper($f, imagecreate(10, 10));
    call_wrapper($f, vec[]);
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



<<__EntryPoint>>
function main_hh_return_type_this_3() :mixed{
error_reporting(-1);
set_error_handler(handler<>);
main();
}
