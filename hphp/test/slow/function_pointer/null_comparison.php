<?hh

function on_notice($_, $errstr, $_, $_, $_, $_) {
  echo "[$errstr] ";
  return true;
}

function wrap($fun) {
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

function comp($what, $x, $y) {
  echo "$what <   :"; wrap(() ==> var_dump($x < $y));
  echo "$what <=  :"; wrap(() ==> var_dump($x <= $y));
  echo "$what >   :"; wrap(() ==> var_dump($x > $y));
  echo "$what >=  :"; wrap(() ==> var_dump($x >= $y));
  echo "$what <=> :"; wrap(() ==> var_dump($x <=> $y));
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
  $clsmeth = class_meth('Foo', 'baz');
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
    foreach ($inner as $gn => $g) {
      comp("$fn x $gn", $f, $g);
      comp("$gn x $fn:", $g, $f);
    }
  }
}
