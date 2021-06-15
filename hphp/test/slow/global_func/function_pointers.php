<?hh

function foo(inout int $x) {
  $x = 42;
}

function inc(int $x): int {
  return $x + 1;
}

function bar(callable $f) {
  return $f('callable check');
}

function call_f((function(int): int) $f) {
  return $f(0);
}

class C {
  function __construct() {
    $this->th = 'th';
  }
  public static function isOdd($i) { return $i % 2 == 1;}
  public function isOddInst($i) { return $i % 2 == 1;}
  public function filter($data)  {
    $callback = inst_meth($this, 'isOddInst');
    return $data->filter($callback);
  }

  public function id<T>(T $x): T {
    return $x;
  }

  public function ref(inout $x) {
    return 0;
  }

  public function meth(inout string $x) {
    $x = $x . $this->th;
    return "inst_" . $x;
  }
}
<<__EntryPoint>>
function main_entry(): void {

  call_user_func(var_dump<>, 1);

  $f = foo<>;
  $x = 0;
  $f(inout $x);
  var_dump($x);

  bar(var_dump<>);
  var_dump(call_f(inc<>));

  $v = Vector {
    Vector {1, 2, 3},
    Vector {1, 2}
  };
  var_dump($v->map(meth_caller('HH\Vector', 'count')));
  var_dump(meth_caller('HH\Vector', 'count')->getClassName());
  var_dump(meth_caller('HH\Vector', 'count')->getMethodName());

  $s = Vector {'1', '2', '3'};
  $data = $s->map(intval<>);
  var_dump($data->filter(class_meth('C', 'isOdd')));
  var_dump((new C)->filter($data));

  $caller = meth_caller(C::class, 'id');
  var_dump($caller(new C(), 'Hello World!'));
  var_dump($caller(new C(), 1337));

  $caller = meth_caller(C::class, 'ref');
  $x = 1;
  try {
    var_dump($caller(new C(), $x));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  $c = new C();
  $meth = inst_meth($c, 'meth');
  $str = 'me';
  var_dump($meth(inout $str));
  var_dump($str);

  print_r($f);
  var_export($f);
  var_dump($f);
  var_dump(json_encode($f));
  $ser = serialize(HH\fun_get_function($f));
  var_dump($ser);
  var_dump(unserialize($ser));
}
