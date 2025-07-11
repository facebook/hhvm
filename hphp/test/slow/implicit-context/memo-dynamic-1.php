<?hh

<<__Memoize, __DynamicallyCallable>>
function memo_normal() :mixed{
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info();
  echo var_dump($hash) . "\n";
}

<<__Memoize(#KeyedByIC), __DynamicallyCallable>>
function memo_keyed() :mixed{
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info();
  if ($hash != null) {
    $str_hash = HH\Lib\Str\join($hash, ', '); // can't do var_dump due to keyedByIC
    echo var_dump($str_hash) . "\n";
  }

}

<<__Memoize(#MakeICInaccessible), __DynamicallyCallable>>
function memo_ic_inaccessible() :mixed{
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info();
  echo var_dump($hash) . "\n";
}

function f(bool $has_ctx) :mixed{
  $v = vec[
    "memo_normal",
    "memo_keyed",
    "memo_ic_inaccessible",
  ];
  foreach ($v as $f) {
    try {
      if ($has_ctx) {
        echo "  Before: " . ClassContext::getContext()->name() . "\n";
      }
    } catch (InvalidOperationException $e) {
      echo $f . " failed with: " . $e->getMessage() . "\n";
    }

    try {
      HH\dynamic_fun($f)();
      echo $f . " passed\n";
    } catch (Exception $e) {
      echo $f . " failed with: " . $e->getMessage() . "\n";
    }

    try {
      if ($has_ctx) {
        echo "  After: " . ClassContext::getContext()->name() . "\n";
      }
    } catch (InvalidOperationException $e) {
      echo $f . " failed with: " . $e->getMessage() . "\n";
    }
    echo "------------------------\n";
  }
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new C(0), () ==> f(true));
  HH\ImplicitContext\soft_run_with(() ==> f(false), "SOFT_KEY");
}
