<?hh


interface IUseMemoize {
  <<__Memoize>>
  public function alwaysMemoize(): int;
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
