<?hh

<<__Memoize(#KeyedByIC)>>
function memo()[zoned] :mixed{
  echo "memo called\n";
}

function f()[zoned] :mixed{
  $key = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo quoted_printable_encode($key) . "\n";
  memo();
}

final class ArraykeyContext extends HH\ImplicitContext {
  const type T = arraykey;
  public static function set($value, $fun) :mixed{ parent::runWith($value, $fun); }
}


<<__EntryPoint>>
function main() :mixed{
  ArraykeyContext::set(123, f<>);
  ArraykeyContext::set("123", f<>);
}
