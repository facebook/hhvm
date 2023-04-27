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

trait T {
  const type T = int;
  public static function set($value, $fun) { parent::runWith($value, $fun); }
}

final class IntContext extends HH\ImplicitContext { use T; }
final class IntContext1 extends HH\ImplicitContext { use T; }

<<__EntryPoint>>
function main() {
  IntContext::set(11, f<>);
  IntContext1::set(1, f<>);
}
