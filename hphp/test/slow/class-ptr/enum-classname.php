<?hh

enum E : string as string {}

<<__EntryPoint>>
function main(): void {
  var_dump(enum_exists(nameof E));
  var_dump(enum_exists("F"));

  var_dump(enum_exists(E::class));
  var_dump(enum_exists(F::class));
  $e = HH\classname_to_class(E::class);
  var_dump(enum_exists($e));
}
