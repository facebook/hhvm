<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function positional(readonly int $r, named int $z = 2): void {
  echo "$r $z\n";
}

class NamedArgs {
  public function __construct(named int $a = 2, named readonly int $b = 3) {
    echo "$a $b\n";
  }
}

function mixed_args(
  readonly int $r,
  named int $a = 2,
  named int $b = 3,
): void {
  echo "$r $a $b\n";
}

<<__NEVER_INLINE>>
function indirect((function(readonly int): void) $f): void {
  $f(readonly 1);
}

<<__EntryPoint>>
function main(): void {
  positional(readonly 1);
  indirect(__hhvm_intrinsics\launder_value(positional<>));
  new NamedArgs(b=readonly 3);
  $class = __hhvm_intrinsics\launder_value(NamedArgs::class);
  new $class(b=readonly 3);
  mixed_args(b=3, readonly 1);
}
