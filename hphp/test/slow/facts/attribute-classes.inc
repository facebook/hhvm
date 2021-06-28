<?hh

interface IBaseForAttributeFiltering {}

class BaseClassForAttributeFiltering implements IBaseForAttributeFiltering {}

class NoArgAttr implements HH\ClassAttribute {}

class TwoArgAttr implements HH\ClassAttribute {
  public function __construct(dynamic $arg0, dynamic $arg1) {}
}

class NoArgMethodAttr implements HH\MethodAttribute {}

class TwoArgMethodAttr implements HH\MethodAttribute {
  public function __construct(dynamic $arg0, dynamic $arg1) {}
}

class NoArgFileAttr implements HH\FileAttribute {}

class TwoArgFileAttr implements HH\FileAttribute {
  public function __construct(dynamic $arg0, dynamic $arg1) {}
}

<<file: NoArgFileAttr, TwoArgFileAttr("Hello", "There")>>

<<TwoArgAttr("apple", "banana")>>
class AppleThenBanana extends BaseClassForAttributeFiltering {}

<<TwoArgAttr("banana", "apple")>>
class BananaThenApple extends BaseClassForAttributeFiltering {}

<<TwoArgAttr("apple", "carrot")>>
class AppleThenCarrot extends BaseClassForAttributeFiltering {}

<<NoArgAttr, TwoArgAttr("abc", "def")>>
class ClassWithTwoAttrs {}

class ClassWithMethodAttrs {

  <<NoArgMethodAttr>>
  public function methodWithNoArgAttr(): void {}

  <<TwoArgMethodAttr(42, "banana")>>
  public function methodWithTwoArgAttr(): void {}
}
