<?hh

<<__Memoize(#KeyedByIC)>>
function memo()[zoned] :mixed{
  echo "memo called\n";
}

function f()[zoned] :mixed{
  $key = HH\ImplicitContext\_Private\get_implicit_context_debug_info();
  $str_hash = HH\Lib\Str\join($key, ', '); // can't do var_dump due to keyedByIC
  echo $str_hash . "\n";
  memo();
}

trait T {
  const type T = int;
  public static function set($value, $fun) :mixed{ parent::runWith($value, $fun); }
}

final class IntContext extends HH\ImplicitContext { use T; }
final class IntContext1 extends HH\ImplicitContext { use T; }

<<__EntryPoint>>
function main() :mixed{
  IntContext::set(11, f<>);
  IntContext1::set(1, f<>);
}
