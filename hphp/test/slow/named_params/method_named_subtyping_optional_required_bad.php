<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

interface OptionalNamedContract {
  public function f(optional named int $value): void;
}

final class RequiredNamedParameter implements OptionalNamedContract {
  public function f(named int $value): void {}
}

<<__EntryPoint>>
function main(): void {
  new RequiredNamedParameter();
}
