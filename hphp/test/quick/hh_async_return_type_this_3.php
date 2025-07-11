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

<<__DynamicallyCallable>> async function f1($p) :Awaitable<mixed>{ return $p; }
<<__DynamicallyCallable>> async function f2($p): Awaitable<int> { return $p; }
<<__DynamicallyCallable>> async function f2_soft($p): <<__Soft>> Awaitable<int> { return $p; }
<<__DynamicallyCallable>> async function f3($p): Awaitable<string> { return $p; }
<<__DynamicallyCallable>> async function f3_soft($p): <<__Soft>> Awaitable<string> { return $p; }
<<__DynamicallyCallable>> async function f4($p): Awaitable<bool> { return $p; }
<<__DynamicallyCallable>> async function f4_soft($p): <<__Soft>> Awaitable<bool> { return $p; }
<<__DynamicallyCallable>> async function f5($p): Awaitable<float> { return $p; }
<<__DynamicallyCallable>> async function f5_soft($p): <<__Soft>> Awaitable<float> { return $p; }
<<__DynamicallyCallable>> async function f6($p): Awaitable<resource> { return $p; }
<<__DynamicallyCallable>> async function f6_soft($p): <<__Soft>> Awaitable<resource> { return $p; }
<<__DynamicallyCallable>> async function f7($p): Awaitable<varray> { return $p; }
<<__DynamicallyCallable>> async function f7_soft($p): <<__Soft>> Awaitable<varray> { return $p; }
<<__DynamicallyCallable>> async function f8($p): Awaitable<callable> { return $p; }
<<__DynamicallyCallable>> async function f8_soft($p): <<__Soft>> Awaitable<callable> { return $p; }
<<__DynamicallyCallable>> async function f9($p): Awaitable<Figure> { return $p; }
<<__DynamicallyCallable>> async function f9_soft($p): <<__Soft>> Awaitable<Figure> { return $p; }
<<__DynamicallyCallable>> async function f10($p): Awaitable<Square> { return $p; }
<<__DynamicallyCallable>> async function f10_soft($p): <<__Soft>> Awaitable<Square> { return $p; }
<<__DynamicallyCallable>> async function f11($p): Awaitable<:div> { return $p; }
<<__DynamicallyCallable>> async function f11_soft($p): <<__Soft>> Awaitable<:div> { return $p; }
<<__DynamicallyCallable>> async function f12($p): Awaitable<Fractal<Square>> { return $p; }
<<__DynamicallyCallable>> async function f12_soft($p): <<__Soft>> Awaitable<Fractal<Square>> { return $p; }
<<__DynamicallyCallable>> async function f13<T>($p): Awaitable<Fractal<T>> { return $p; }
<<__DynamicallyCallable>> async function f13_soft<T>($p): <<__Soft>> Awaitable<Fractal<T>> { return $p; }
<<__DynamicallyCallable>> async function f14($p): Awaitable<my_t> { return $p; }
<<__DynamicallyCallable>> async function f14_soft($p): <<__Soft>> Awaitable<my_t> { return $p; }
<<__DynamicallyCallable>> async function f15($p): Awaitable<void> { return $p; }
<<__DynamicallyCallable>> async function f15_soft($p): <<__Soft>> Awaitable<void> { return $p; }
<<__DynamicallyCallable>> async function f16($p): Awaitable<mixed> { return $p; }
<<__DynamicallyCallable>> async function f16_soft($p): <<__Soft>> Awaitable<mixed> { return $p; }
<<__DynamicallyCallable>> async function f17($p): Awaitable<?int> { return $p; }
<<__DynamicallyCallable>> async function f17_soft($p): <<__Soft>> Awaitable<?int> { return $p; }
<<__DynamicallyCallable>> async function f18($p): Awaitable<(string, int)> { return $p; }
<<__DynamicallyCallable>> async function f18_soft($p): <<__Soft>> Awaitable<(string, int)> { return $p; }
<<__DynamicallyCallable>> async function f19($p): Awaitable<(function(int): int)> { return $p; }
<<__DynamicallyCallable>> async function f19_soft($p): <<__Soft>> Awaitable<(function(int): int)> { return $p; }
<<__DynamicallyCallable>> async function f20($p): Awaitable<callable> { return $p; }
<<__DynamicallyCallable>> async function f20_soft($p): <<__Soft>> Awaitable<callable> { return $p; }

class Figure {}
class Square extends Figure {}
class Fractal<T> extends Figure {}
class :div {}

class A {}
class B extends A {
  public async function f21(): Awaitable<this> { return $this; }
  public async function f21_soft(): <<__Soft>> Awaitable<this> { return $this; }
  public async function f22($p): Awaitable<this> { return $p; }
  public async function f22_soft($p): <<__Soft>> Awaitable<this> { return $p; }
  public static function testfunc() :mixed{}
}
class C extends B {}

function testfunc() :mixed{}

function call_wrapper($fn, $arg) :mixed{
  try {
    HH\Asio\join($fn($arg));
  } catch (Exception $e) {
    echo "Caught exception: " . $e->getMessage() . "\n";
  }
}

function main() :mixed{
  for ($i = 1; $i <= 20; $i++) {
    foreach (vec['', '_soft'] as $suffix) {
      if ($suffix !== '' && $i === 1) {
        // f1_soft() does not exist
        continue;
      }
      $f = 'f' . $i . $suffix;
      echo "\ncalling $f\n";
      call_wrapper(HH\dynamic_fun($f), null);
      call_wrapper(HH\dynamic_fun($f), 42);
      call_wrapper(HH\dynamic_fun($f), 'foobar');
      call_wrapper(HH\dynamic_fun($f), true);
      call_wrapper(HH\dynamic_fun($f), 14.1);
      call_wrapper(HH\dynamic_fun($f), imagecreate(10, 10));
      call_wrapper(HH\dynamic_fun($f), vec[]);
      call_wrapper(HH\dynamic_fun($f), function($x){return $x*$x;});
      call_wrapper(HH\dynamic_fun($f), new Figure());
      call_wrapper(HH\dynamic_fun($f), new Square());
      call_wrapper(HH\dynamic_fun($f), new Fractal());
      call_wrapper(HH\dynamic_fun($f), <div/>);
      call_wrapper(HH\dynamic_fun($f), 'testfunc');
      call_wrapper(HH\dynamic_fun($f), vec['C', 'testfunc']);
    }
  }

  $c = new C();
  echo "\ncalling f21\n";
  try {
    HH\Asio\join($c->f21());
  } catch (Exception $e) {
    echo "Caught exception: " . $e->getMessage() . "\n";
  }
  try {
    HH\Asio\join($c->f21_soft());
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
function main_entry(): void {


  error_reporting(-1);
  set_error_handler(handler<>);
  main();
}
