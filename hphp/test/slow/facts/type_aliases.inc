<?hh

class TypeAliasAttr extends HH\TypeAliasAttribute {
  public function __construct(
    private int $arg1,
    private string $arg2,
  ) {
  }
}

type SomeTypeAlias = IBase;

<<TypeAliasAttr(42, 'a')>>
type TypeAliasWithAttr = int;
