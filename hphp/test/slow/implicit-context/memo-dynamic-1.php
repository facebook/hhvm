<?hh

<<__Memoize>>
function memo_normal() :mixed{
  $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo quoted_printable_encode($hash) . "\n";
}

<<__Memoize(#KeyedByIC)>>
function memo_keyed() :mixed{
  $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo quoted_printable_encode($hash) . "\n";
}

<<__Memoize(#MakeICInaccessible)>>
function memo_ic_inaccessible() :mixed{
  $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo quoted_printable_encode($hash) . "\n";
}

<<__Memoize(#SoftMakeICInaccessible)>>
function memo_soft_ic_inaccessible() :mixed{
  $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo quoted_printable_encode($hash) . "\n";
}

function f(bool $has_ctx) :mixed{
  $v = vec[
    "memo_normal",
    "memo_keyed",
    "memo_ic_inaccessible",
    "memo_soft_ic_inaccessible",
  ];
  foreach ($v as $f) {
    if ($has_ctx) {
      echo "  Before: " . ClassContext::getContext()->name() . "\n";
    }
    try {
      $f();
      echo $f . " passed\n";
    } catch (Exception $e) {
      echo $f . " failed with: " . $e->getMessage() . "\n";
    }
    if ($has_ctx) {
      echo "  After: " . ClassContext::getContext()->name() . "\n";
    }
    echo "------------------------\n";
  }
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new C, () ==> f(true));
  HH\ImplicitContext\soft_run_with(() ==> f(false), "SOFT_KEY");
}
