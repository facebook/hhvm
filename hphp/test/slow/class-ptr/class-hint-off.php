<?hh

<<file:__EnableUnstableFeatures('class_type')>>

function param_class(class<C> $c): void {}
function param_classname(classname<C> $cn): void {}
function ret_class(mixed $m): class<C> {
  return $m;
}
function ret_classname(mixed $m): classname<C> {
  return $m;
}

class C {
  public static ?class<C> $c = null;
  public static ?class<C> $cn = null;
}

function caller(mixed $m): void {
  __hhvm_intrinsics\debug_var_dump_lazy_class($m);
  echo "=> parameter\n";
  param_class($m);
  param_classname($m);

  echo "=> return\n";
  ret_class($m);
  ret_classname($m);

  echo "=> prop\n";
  C::$c = $m;
  C::$cn = $m;
}

function launder_caller(mixed $m): void {
  echo "Laundering ";
  caller(__hhvm_intrinsics\launder_value($m));
}

<<__EntryPoint>>
function main(): void {
  $s = nameof C;
  $l = C::class;
  $c = HH\classname_to_class($l);

  caller($s);
  caller($l);
  caller($c);

  launder_caller($s);
  launder_caller($l);
  launder_caller($c);
}
