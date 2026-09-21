<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

interface NamedContract {
  public function f(named int $z): void;
}

final class MissingNamedParameter implements NamedContract {
  public function f(named int $a = 0): void {}
}

<<__EntryPoint>>
function main(): void {
  new MissingNamedParameter();
}
