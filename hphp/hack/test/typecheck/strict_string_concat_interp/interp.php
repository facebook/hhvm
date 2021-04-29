<?hh // strict

// These should all have typing errors.
function bad_interp(bool $a, float $b, null $c, string $d): void {
  "interp $a";
  "interp $b";
  "interp $c";
  "interp $d $a";
}

// These should typecheck fine.
function ok_interp(
  string $a,
  int $b,
  \HH\FormatString<\PlainSprintf> $c,
  dynamic $d
): void {
  "interp $a";
  "interp $b";
  "interp $c";
  "interp $d";
  "interp $a $b $c $d";
}
