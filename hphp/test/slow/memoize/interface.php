<?hh // strict


interface IUseMemoize {
  <<__Memoize>>
  public function alwaysMemoize(): int;
}
