<?hh // partial

interface ISuper {}

interface IMarked {
  require implements ISuper;

  public function methodOfMarked(): int;
}
