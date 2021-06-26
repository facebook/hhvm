<?hh
class C {
  <<__DynamicallyCallable>> static public function func1() {
    return 1;
  }
}

class D extends C {
}

function typehint(callable $m) {
  \var_dump($m, $m());
}

function check($m) {
  $n = null;
  \var_dump($m, is_callable($m));
  \var_dump(is_callable($m, true));
  \var_dump(is_callable_with_name($m, false, inout $n));
  \var_dump($n);
  \var_dump(is_callable_with_name($m, true, inout $n));
  \var_dump($n);
  \var_dump(is_callable_with_name($m, false, inout $n));
  \var_dump($n);
  \var_dump(is_callable_with_name($m, true, inout $n));
  \var_dump($n);
}

<<__EntryPoint>>
function main() {
  typehint(vec[D::class, 'func1']);
  check(vec[D::class, 'func1']);
  typehint(HH\class_meth(D::class, 'func1'));
  check(HH\class_meth(D::class, 'func1'));
}
