<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

interface PositionalContract {
  public function f(int $value): void;
}

final class MissingPositionalParameter implements PositionalContract {
  public function f(named int $extra = 0): void {}
}

<<__EntryPoint>>
function main(): void {
  new MissingPositionalParameter();
}
