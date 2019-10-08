<?hh // strict
interface IFoo {
  public static function test(): string;
}

class Bar implements IFoo {
  final public static function test(): string {
    return "a";
  }
}
