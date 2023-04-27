<?hh

enum class PessEnumClassSupportDynBound : mixed {
  int ONE = 1;
  string TWO = 'two';
}

enum class PessEnumClass : arraykey {
  int ONE = 1;
  string TWO = 'two';
}

interface IHasName {
  public function name(): string;
}

abstract enum class PessAbstracEnumClas: IHasName {
  abstract HasName Foo;
  HasName Bar = new HasName('bar');
}

enum PessRegularEnum : string {
  Yes = 'Y';
  No = 'N';
}

enum PessRegularEnumBound : arraykey as arraykey {
  Yes = 'Y';
  No = 42;
}
