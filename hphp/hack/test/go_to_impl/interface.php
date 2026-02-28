<?hh
interface IFoo {
  public static function test(): string;
}

interface IBar extends IFoo {
  public static function test(): string;
}

class Baz implements IBar {
  final public static function test(): string {
    return "a";
  }
}
