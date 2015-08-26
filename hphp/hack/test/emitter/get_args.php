<?hh

function foo(): void {
  var_dump(func_get_args());
}

function test(): void {
  /* HH_FIXME[4105] */
  foo(1,2,3);
}
