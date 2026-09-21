<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class InoutBase {
  public function f(inout int $value): void {}
}

final class InoutModeMismatch extends InoutBase {
  public function f(named bool $extra = false, int $value): void {}
}

<<__EntryPoint>>
function main(): void {
  new InoutModeMismatch();
}
