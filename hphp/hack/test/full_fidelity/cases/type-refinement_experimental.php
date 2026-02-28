<?hh

interface I {}

type InAlias = I with { type T as int; ctx C = []; };

function in_fun_hint(
  (function(I with { type T = int }): string) $_,
): void {}

interface F {
  public function in_abstr_method_return_type(
  ): Box with { ctx C super [globals] };
}
