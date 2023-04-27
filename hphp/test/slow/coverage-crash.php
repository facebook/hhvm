<?hh

<<__NEVER_INLINE>>
function foo() {
  var_dump(__FUNCTION__);
}

<<__NEVER_INLINE>>
function bar() {
  var_dump(__FUNCTION__);
  foo();
}

<<__EntryPoint>>
function main() {
  var_dump(__FUNCTION__);
  bar();

  HH\enable_function_coverage();
}
