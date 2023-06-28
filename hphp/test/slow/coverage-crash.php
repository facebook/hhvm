<?hh

<<__NEVER_INLINE>>
function foo() :mixed{
  var_dump(__FUNCTION__);
}

<<__NEVER_INLINE>>
function bar() :mixed{
  var_dump(__FUNCTION__);
  foo();
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(__FUNCTION__);
  bar();

  HH\enable_function_coverage();
}
