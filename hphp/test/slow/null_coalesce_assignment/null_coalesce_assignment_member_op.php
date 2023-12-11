<?hh

class A {
  public ?int $foo = null;
  public static function fun(): void {}
}

class B {
  public function __construct() {
    echo "object created!"."\n";
  }
}

function side_effect(): int {
  echo "mutation!"."\n";
  return 5;
}

function VS($x, $y) :mixed{
  var_dump($x === $y);
  if ($x !== $y) {
    echo "FAILED: $y\n";
  }
}

<<__EntryPoint>>
function main() :mixed{
  // Dict
  $b = dict['a' => dict['b' => 1]];
  $b['a']['b'] ??= 17;
  VS($b['a']['b'], 1);

  $f = dict[];
  $f['f'] ??= 15;
  VS($f['f'], 15);

  $e = dict[];
  $e['e'] ??= dict[];
  $e['e']['a'] ??= 15;
  VS($e['e']['a'], 15);

  $a = dict['a' => null];
  $a['a'] ??= dict[];
  $a['a']['b'] = 10;

  // Uninit
  $c ??= 17;
  VS($c, 17);

  try {
    $c1['a'] ??= 17;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  // Init
  $d = 10;
  $d ??= 17;
  VS($d, 10);

  // Object
  $g = new A();
  $g->foo ??= 10;
  VS($g->foo, 10);

  // String
  $j = "one";
  $j[0] ??= "t";
  VS($j, "one");

  // String OOB
  $j[4] ??= "t";
  VS($j, "one");

  // Null
  try {
    $l = null;
    $l[0] ??= "two";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  // Int
  $o = 10;
  $o[0] ?? 1;
  VS($o, 10);

  // Class
  $class = A::class;
  var_dump($class);
  $class[0] ??= 'g';
  VS($class, A::class);

  // Clsmeth
  $clsmeth = A::fun<>;
  $clsmeth[0] ??= 10;

  // Short-circuit
  $i = 0;
  $arr = dict[];
  VS($arr[$i] ??= ++$i, 1);
  VS($i, 1);
  VS($arr[0] ??= ++$i, 1);
  VS($i, 1);

  $k = 10;
  $k ??= side_effect();

  // New operator
  $j ??= new B();

  $j1 = new B();
  $j1 ??= new B();

  // False
  try {
    $x = false;
    $x[0] ??= 't';
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  // True
  $x = true;
  $x[0] ??= 't';
  VS($x, true);

  // Object
  $m = Vector {1, 2};
  $m[0] ??= "one";
  try {
    $m[2] ??= "one";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
