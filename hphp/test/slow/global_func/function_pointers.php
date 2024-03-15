<?hh

function foo(inout int $x) :mixed{
  $x = 42;
}

function inc(int $x): int {
  return $x + 1;
}

function bar(callable $f) :mixed{
  return $f('callable check');
}

function call_f((function(int): int) $f) :mixed{
  return $f(0);
}

class C {
  function __construct() {
    $this->th = 'th';
  }
  public static function isOdd($i) :mixed{ return $i % 2 == 1;}
  public function isOddInst($i) :mixed{ return $i % 2 == 1;}
  public function filter($data)  :mixed{
    $callback = ($i) ==> $this->isOddInst($i);
    return $data->filter($callback);
  }

  public function id<T>(T $x): T {
    return $x;
  }

  public function ref(inout $x) :mixed{
    return 0;
  }

  public function meth(inout string $x) :mixed{
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
  var_dump($v->map(meth_caller(Vector::class, 'count')));
  var_dump(meth_caller(Vector::class, 'count')->getClassName());
  var_dump(meth_caller(Vector::class, 'count')->getMethodName());

  $s = Vector {'1', '2', '3'};
  $data = $s->map(intval<>);
  var_dump($data->filter(C::isOdd<>));
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

  print_r($f);
  var_export($f);
  var_dump($f);
  var_dump(json_encode($f));
  $ser = serialize(HH\fun_get_function($f));
  var_dump($ser);
  var_dump(unserialize($ser));
}
