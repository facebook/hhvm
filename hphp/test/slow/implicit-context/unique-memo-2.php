<?hh

<<__Memoize(#KeyedByIC)>>
function memo()[zoned] {
  echo "memo called\n";
}

function f()[zoned] {
  $key = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo quoted_printable_encode($key) . "\n";
  memo();
}

final class ArraykeyContext extends HH\ImplicitContext {
  const type T = arraykey;
  public static function set($value, $fun) { parent::runWith($value, $fun); }
}


<<__EntryPoint>>
function main() {
  ArraykeyContext::set(123, f<>);
  ArraykeyContext::set("123", f<>);
}
