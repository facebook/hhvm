<?hh

class C {
  public static function bar<reify T1, reify T2>() {
    return 42;
  }
}

function foo<reify T>() {
  $meth = C::bar<T, int>;
  gc_collect_cycles();
  gc_collect_cycles();
  return $meth();
}

<<__EntryPoint>>
function main() {
  var_dump(foo<string>());
}
