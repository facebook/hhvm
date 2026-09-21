<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

interface NamedContract {
  public function f(named int $z): void;
}

final class ExtraRequiredNamedParameter implements NamedContract {
  public function f(named int $a, named int $z): void {}
}

<<__EntryPoint>>
function main(): void {
  new ExtraRequiredNamedParameter();
}
