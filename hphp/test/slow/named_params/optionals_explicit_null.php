<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function show_value(?int $value): string {
  return $value is null ? 'null' : (string)$value;
}

function named_values(named ?int $a = 7, named ?int $b = 17): string {
  return show_value($a).','.show_value($b);
}

function reified_variadic_values<reify T>(
  named ?int $a = 7,
  named ?int $b = 17,
  mixed ...$rest
): string {
  return named_values(a=$a, b=$b).','.count($rest);
}

function positional_values(?int $a = 7, ?int $b = 17): string {
  return show_value($a).','.show_value($b);
}

<<__EntryPoint>>
function main(): void {
  var_dump(named_values());
  var_dump(named_values(b=null));
  var_dump(named_values(b=23));
  var_dump(named_values(a=7, b=null));
  var_dump(named_values(a=null));
  var_dump(reified_variadic_values<int>(b=null, 23, 24));
  var_dump(positional_values());
  var_dump(positional_values(7, null));
}
