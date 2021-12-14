<?hh

final class ReadonlyA {
  protected function __construct(private readonly (readonly function(): void) $handler) {}
  public readonly function foo(): void {
    // do stuff
    ($this->handler)();
  }
  public static function get(): ReadonlyA {
    return new ReadonlyA(self::defaultHandler<>); // line X
  }
  public static function defaultHandler(): void {
    echo "hi!\n";
  } // line Y
}

<<__EntryPoint>>
function foo(): void {
  $x = ReadonlyA::get();
  $x->foo();
}
