<?hh // strict

function foo(): void {
  /* HH_FIXME[2049] */
  /* HH_FIXME[4107] */
  var_dump(func_get_args());
}

function test(): void {
  /* HH_FIXME[4105] */
  foo(1,2,3);
}
