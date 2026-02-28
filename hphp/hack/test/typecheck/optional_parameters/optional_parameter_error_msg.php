<?hh

function takes_func_with_optional_param(
  (function(int, optional int): void) $f,
): void {}

function test(): void {
  takes_func_with_optional_param(($x, $y) ==> {});
  takes_func_with_optional_param(($x) ==> {});
}
