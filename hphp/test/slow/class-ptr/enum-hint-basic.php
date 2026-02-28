<?hh

<<file:__EnableUnstableFeatures('class_type')>>

function param_enum(enum<E> $c): void {}
function param_enumname(HH\enumname<E> $cn): void {}

enum E: int {
  A = 1;
}

function caller(mixed $m): void {
  __hhvm_intrinsics\debug_var_dump_lazy_class($m);
  echo "=> parameter\n";
  param_enum($m);
  param_enumname($m);
}

function launder_caller(mixed $m): void {
  echo "Laundering ";
  caller(__hhvm_intrinsics\launder_value($m));
}

<<__EntryPoint>>
function main(): void {
  $s = nameof E;
  $l = E::class;
  $c = HH\classname_to_class($l);

  caller($s);
  caller($l);
  caller($c);

  launder_caller($s);
  launder_caller($l);
  launder_caller($c);
}
