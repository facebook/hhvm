<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function show_variadic_fun_type_no_other_args((function(...): void) $f): void {
  hh_show($f);
}
function show_variadic_fun_type((function(int, ...): void) $f): void {
  hh_show($f);
}
