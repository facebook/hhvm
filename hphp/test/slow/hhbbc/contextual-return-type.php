<?hh

interface IFoo<+T> {}

abstract class Foo {
  final public static function bar<T>(IFoo<T> $p): IFoo<T> {
    return $p as IFoo<_>;
  }
}

class Baz implements IFoo<string> {}
class Baz2 implements IFoo<string> {}

enum class EFoo: IFoo {
  IFoo<string> cns = Foo::bar(new Baz());
}

<<__EntryPoint>>
function main() {
  echo "DONE!\n";
}
