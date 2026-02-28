<?hh

<<__EntryPoint>>
function main_duplicate_require2() :mixed{
  $n = __hhvm_intrinsics\apc_fetch_no_check('foo');
  if (!$n) $n = 0;
  apc_store('foo', ++$n);

  echo "Iteration: $n\n";

  if ($n == 1) {
    include 'duplicate_require2-1.inc';
  } else if ($n == 2) {
    include 'duplicate_require2-2.inc';
  }

  if ($n & 1) {
    include 'duplicate_require2-3.inc';
  }

  if ($n & 2) {
    include 'duplicate_require2-4.inc';
  }

  if ($n == 3) {
    include 'duplicate_require2-5.inc';
  }
}
