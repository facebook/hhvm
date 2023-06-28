<?hh

function on_notice($_, $errstr, $_, $_, $_, $_) :mixed{
  echo "[$errstr] ";
  return true;
}

function wrap($fun) :mixed{
  try {
    $fun();
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}

class Foo {
  public static function bar<reify T>(): void {}
  public static function baz(): void {}
}

function is_reified($x) :mixed{
  return !HH\is_fun($x) && !HH\is_class_meth($x) && !($x is Closure) && !($x is null);
}

function comp($what, $x, $y) :mixed{
  $either_reified = is_reified($x) || is_reified($y);
  echo "$what <   :"; wrap(() ==> var_dump($either_reified ? $x < $y   : HH\Lib\Legacy_FIXME\lt($x, $y)));
  echo "$what <=  :"; wrap(() ==> var_dump($either_reified ? $x <= $y  : HH\Lib\Legacy_FIXME\lte($x, $y)));
  echo "$what >   :"; wrap(() ==> var_dump($either_reified ? $x > $y   : HH\Lib\Legacy_FIXME\gt($x, $y)));
  echo "$what >=  :"; wrap(() ==> var_dump($either_reified ? $x >= $y  : HH\Lib\Legacy_FIXME\gte($x, $y)));
  echo "$what <=> :"; wrap(() ==> var_dump($either_reified ? $x <=> $y : HH\Lib\Legacy_FIXME\cmp($x, $y)));
  print("\n");
}

function foo<reify T>(): void {}
function bar(): void {}
class C {}

<<__EntryPoint>>
function main(): void {
  set_error_handler(on_notice<>, E_NOTICE);

  $rf = foo<int>;
  $fp = bar<>;
  $lambda = () ==> 1;
  $fun = bar<>;
  $clsmeth = Foo::baz<>;
  $rclsmeth = Foo::bar<int>;

  $outer = dict[
    "func_ptr" => $fp,
    "rfunc_ptr" => $rf,
    "clsmeth" => $clsmeth,
    "rclsmeth" => $rclsmeth,
    "lambda" => $lambda,
    "fun" => $fun
  ];
  $inner = dict["null" => null];

  foreach ($outer as $fn => $f) {
    comp("$fn x null", $f, null);
    comp("null x $fn:", null, $f);
  }
}
