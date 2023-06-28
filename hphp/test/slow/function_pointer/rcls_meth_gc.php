<?hh

class C {
  public static function bar<reify T1, reify T2>() :mixed{
    return 42;
  }
}

function foo<reify T>() :mixed{
  $meth = C::bar<T, int>;
  gc_collect_cycles();
  gc_collect_cycles();
  return $meth();
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(foo<string>());
}
