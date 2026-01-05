<?hh
<<file: __EnableUnstableFeatures('type_splat', 'open_tuples')>>

class C<T as (mixed...)> {}

function control(?string $s = null): void {}

function req0_opt1_splat<T as (mixed...)>(
  ?string $s = null,
  ... T $args,
): C<T> {
  return new C();
}
function req1_opt1_splat<T as (mixed...)>(
  int $x,
  bool $b = false,
  ... T $args,
): C<T> {
  return new C();
}
function req0_opt1_splat_opt<T as (int, mixed...)>(
  ?string $s = null,
  ... T $args,
): C<T> {
  return new C();
}

function main(): void {
  control();
  control("A");

  $a0 = req0_opt1_splat();
  hh_expect<C<()>>($a0);
  $a1 = req0_opt1_splat("a");
  hh_expect<C<()>>($a1);
  $a2 = req0_opt1_splat("A", 23);
  hh_expect<C<(int)>>($a2);
  $a3 = req0_opt1_splat("A", 23, false);
  hh_expect<C<(int, bool)>>($a3);
  // Should error
  $a4 = req1_opt1_splat();

  $a5 = req1_opt1_splat(23);
  hh_expect<C<()>>($a5);
  $a6 = req1_opt1_splat(23, true);
  hh_expect<C<()>>($a6);
  $a7 = req1_opt1_splat(23, true, 2.3);
  hh_expect<C<(float)>>($a7);

  // Illegal because there is a required parameter in the splat bound
  $a8 = req0_opt1_splat_opt();
  // Illegal because there is a required parameter in the splat bound
  $a9 = req0_opt1_splat_opt("a");
  // Legal because we have provided the parameter required for the splat bound
  $a10 = req0_opt1_splat_opt("A", 14);
}
