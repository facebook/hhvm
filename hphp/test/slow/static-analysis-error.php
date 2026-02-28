<?hh

<<__NEVER_INLINE>>
function foo(bool $b): void {
  if ($b) __hhvm_intrinsics\static_analysis_error();
}

function bar(bool $b): void {
  foo($b);
}

function biz(bool $b): void {
  bar($b);
}

<<__NEVER_INLINE>>
function buz(bool $b) {
  biz($b);
}

<<__EntryPoint>>
function main() {
  buz(false);
  buz(false);
  buz(false);
  buz(false);
  buz(false);
  buz(true);
}
