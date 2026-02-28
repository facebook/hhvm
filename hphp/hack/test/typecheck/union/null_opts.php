<?hh

function f(
  bool $b,
  nonnull $nonnull,
  ?int $nullableInt,
  ?int $nullableInt2,
  ?string $nullableString,
  ?bool $nullableBool,
): void {

  $x = ($b ? null : null);
  hh_show($x);

  $x = ($b ? null : 1);
  hh_show($x);

  $x = ($b ? null : $nonnull);
  hh_show($x);

  $x = ($b ? 1 : $nullableInt);
  hh_show($x);

  $x = ($b ? null : $nullableInt);
  hh_show($x);

  $x = ($b ? "" : $nullableInt);
  hh_show($x);

  $x = ($b ? $nullableInt : $nullableString);
  hh_show($x);

  $x = ($b ? $nullableInt : ($b ? $nullableString : $nullableBool));
  hh_show($x);
}
