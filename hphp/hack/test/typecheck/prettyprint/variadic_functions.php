<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function show_variadic_fun_type_no_other_args(
  (function(mixed...): void) $f,
): void {
  hh_show($f);
}
function show_variadic_fun_type((function(int, mixed...): void) $f): void {
  hh_show($f);
}
function show_variadic_fun_type_no_other_args_typed(
  (function(int...): void) $f,
): void {
  hh_show($f);
}
function show_variadic_fun_type_typed(
  (function(int, string...): void) $f,
): void {
  hh_show($f);
}
