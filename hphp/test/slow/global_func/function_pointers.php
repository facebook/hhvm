<?hh

call_user_func(fun('var_dump'), 1);

function foo(inout int $x) {
  $x = 42;
}

function inc(int $x): int {
  return $x + 1;
}

$f = fun('foo');
$x = 0;
$f(inout $x);
var_dump($x);

function bar(callable $f) {
  return $f('callable check');
}

function call_f((function(int): int) $f) {
  return $f(0);
}

bar(fun('var_dump'));
var_dump(call_f(fun('inc')));

$v = Vector {
  Vector {1, 2, 3},
  Vector {1, 2}
};
var_dump($v->map(meth_caller('HH\Vector', 'count')));
var_dump(meth_caller('HH\Vector', 'count')->getClassName());
var_dump(meth_caller('HH\Vector', 'count')->getMethodName());

class C {
  function __construct() {
    $this->th = 'th';
  }
  public static function isOdd($i) { return $i % 2 == 1;}
  public function filter($data)  {
    $callback = inst_meth($this, 'isOdd');
    return $data->filter($callback);
  }

  public function id<T>(T $x): T {
    return $x;
  }

  public function ref(&$x = null) {
    return 0;
  }

  public function meth(inout string $x) {
    $x = $x . $this->th;
    return "inst_" . $x;
  }
}

$s = Vector {'1', '2', '3'};
$data = $s->map(fun('intval'));
var_dump($data->filter(class_meth('C', 'isOdd')));
var_dump((new C)->filter($data));

$caller = meth_caller(C::class, 'id');
var_dump($caller(new C(), 'Hello World!'));
var_dump($caller(new C(), 1337));

$caller = meth_caller(C::class, 'ref');
$x = 1;
var_dump($caller(new C()));
var_dump($caller(new C(), $x));

$c = new C();
$meth = inst_meth($c, 'meth');
$str = 'me';
var_dump($meth(inout $str));
var_dump($str);

print_r($f);
var_export($f);
var_dump($f);
var_dump(json_encode($f));
$ser = serialize($f);
var_dump($ser);
var_dump(unserialize($ser));
